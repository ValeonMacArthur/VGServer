package router

import (
	"AuthorVGroup/models"
	"bytes"
	"encoding/json"
	"errors"
	"io/ioutil"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/golang-jwt/jwt/v5"
	"gorm.io/gorm"
)

const (
	appleTeamID   = "YOUR_TEAM_ID"
	appleClientID = "com.example.app" // 你的服务ID
	appleAuthURL  = "https://appleid.apple.com/auth/token"
	applePrivKey  = `-----BEGIN PRIVATE KEY-----
YOUR_PRIVATE_KEY_CONTENT
-----END PRIVATE KEY-----`
)

type AppleTokenResponse struct {
	AccessToken  string `json:"access_token"`
	ExpiresIn    int    `json:"expires_in"`
	IDToken      string `json:"id_token"`
	RefreshToken string `json:"refresh_token"`
	TokenType    string `json:"token_type"`
}

type AppleIDTokenClaims struct {
	Sub   string `json:"sub"`   // Apple 用户唯一ID
	Email string `json:"email"` // 用户邮箱
	jwt.RegisteredClaims
}

// 请求参数结构
type AppleLoginRequest struct {
	Code     string `json:"code" binding:"required"` // 前端传来的 authorization code
	DeviceID string `json:"device_id,omitempty"`     // 可选设备ID
}

func AppleLoginHandler(c *gin.Context) {
	var req AppleLoginRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "缺少code参数"})
		return
	}

	// 1. 用 authorization code 去苹果换token
	appleTokenResp, err := requestAppleToken(req.Code)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "Apple授权失败"})
		return
	}

	// 2. 解析 id_token 得到用户信息
	claims, err := parseAppleIDToken(appleTokenResp.IDToken)
	if err != nil {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "无效的Apple ID Token"})
		return
	}

	appleUserID := claims.Sub
	email := claims.Email

	// 3. 查 Identity 表是否有该用户
	var identity models.Identity
	err = models.DB.Where("identity_type = ? AND identifier = ?", "apple", appleUserID).First(&identity).Error
	if err != nil && !errors.Is(err, gorm.ErrRecordNotFound) {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}

	var user models.User
	if err == nil {
		// 找到身份对应的用户
		err = models.DB.Where("user_id = ?", identity.UserID).First(&user).Error
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
			return
		}
	} else {
		// 未找到，注册新用户
		deviceID := req.DeviceID
		ip := c.ClientIP()

		err = models.DB.Transaction(func(tx *gorm.DB) error {
			emailCopy := email
			newUser := models.User{
				Email:    &emailCopy,
				Role:     1,
				IsActive: true,
			}
			if err := tx.Create(&newUser).Error; err != nil {
				return err
			}

			newIdentity := models.Identity{
				UserID:       newUser.UserID,
				IdentityType: "apple",
				Identifier:   appleUserID,
				IsVerified:   true,
			}
			if err := tx.Create(&newIdentity).Error; err != nil {
				return err
			}

			var devicePtr *string
			if deviceID != "" {
				devicePtr = &deviceID
			}
			ipPtr := &ip

			risk := models.UserRisk{
				UserID:      newUser.UserID,
				DeviceID:    devicePtr,
				IPAddress:   ipPtr,
				RiskLevel:   0,
				RiskType:    "apple_login",
				Status:      "pending",
				TriggeredAt: time.Now(),
			}
			if err := tx.Create(&risk).Error; err != nil {
				return err
			}

			user = newUser
			return nil
		})
		if err != nil {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "注册失败"})
			return
		}
	}

	// 4. 生成 JWT 并返回
	accessToken, err := generateAccessToken(*user.Email, user.UserID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "生成访问令牌失败"})
		return
	}
	refreshToken, err := generateRefreshToken(*user.Email, user.UserID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "生成刷新令牌失败"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"access_token":  accessToken,
		"refresh_token": refreshToken,
		"message":       "登录成功",
	})
}

// 用authorization code请求Apple换token
func requestAppleToken(code string) (*AppleTokenResponse, error) {
	clientSecret, err := generateAppleClientSecret()
	if err != nil {
		return nil, err
	}

	values := map[string]string{
		"client_id":     appleClientID,
		"client_secret": clientSecret,
		"code":          code,
		"grant_type":    "authorization_code",
	}

	jsonValue, _ := json.Marshal(values)
	req, err := http.NewRequest("POST", appleAuthURL, bytes.NewBuffer(jsonValue))
	if err != nil {
		return nil, err
	}
	req.Header.Set("Content-Type", "application/json")

	resp, err := http.DefaultClient.Do(req)
	if err != nil {
		return nil, err
	}
	defer resp.Body.Close()
	bodyBytes, _ := ioutil.ReadAll(resp.Body)

	if resp.StatusCode != http.StatusOK {
		return nil, errors.New("Apple授权服务器返回错误: " + string(bodyBytes))
	}

	var tokenResp AppleTokenResponse
	if err := json.Unmarshal(bodyBytes, &tokenResp); err != nil {
		return nil, err
	}

	return &tokenResp, nil
}

// 生成 Apple Client Secret JWT
func generateAppleClientSecret() (string, error) {
	now := time.Now()
	claims := jwt.RegisteredClaims{
		Issuer:    appleTeamID,
		IssuedAt:  jwt.NewNumericDate(now),
		ExpiresAt: jwt.NewNumericDate(now.Add(10 * time.Minute)),
		Audience:  []string{"https://appleid.apple.com"},
		Subject:   appleClientID,
	}

	token := jwt.NewWithClaims(jwt.SigningMethodES256, claims)
	privateKey, err := jwt.ParseECPrivateKeyFromPEM([]byte(applePrivKey))
	if err != nil {
		return "", err
	}

	return token.SignedString(privateKey)
}

// 解析并验证 Apple 返回的 id_token
func parseAppleIDToken(idToken string) (*AppleIDTokenClaims, error) {
	token, err := jwt.ParseWithClaims(idToken, &AppleIDTokenClaims{}, func(token *jwt.Token) (interface{}, error) {
		// TODO: 这里要从Apple的公开密钥中获取公钥验证签名，或者使用第三方库
		// 简化示例，暂时不验证签名，生产环境必须验证！
		return nil, nil
	})

	if err != nil {
		return nil, err
	}

	if claims, ok := token.Claims.(*AppleIDTokenClaims); ok && token.Valid {
		if !containsAudience(claims.Audience, appleClientID) {
			return nil, errors.New("audience不匹配")
		}
		return claims, nil
	}

	return nil, errors.New("无效的id_token")
}

func containsAudience(audiences []string, target string) bool {
	for _, aud := range audiences {
		if aud == target {
			return true
		}
	}
	return false
}
