package router

import (
	"AuthorVGroup/hiredis"
	"AuthorVGroup/models"
	"crypto/rand"
	"errors"
	"fmt"
	"net/http"
	"net/smtp"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/go-redis/redis/v8"
	"github.com/golang-jwt/jwt/v5"
	"gorm.io/gorm"
)

const GoogleEmailPwd = "pkzp gfot loet pcvq"

// 生成6位数字验证码
func GenerateCode() string {
	const digits = "0123456789"
	b := make([]byte, 6)
	rand.Read(b)
	for i := 0; i < 6; i++ {
		b[i] = digits[int(b[i])%len(digits)]
	}
	return string(b)
}

// 发送验证码邮件（Gmail版本）
func SendVerificationCodeGmail(toEmail, code string) error {
	from := "dojim590@gmail.com" // 你的Gmail邮箱
	password := GoogleEmailPwd   // 你在Google生成的“应用专用密码”
	smtpHost := "smtp.gmail.com"
	smtpPort := "587"

	auth := smtp.PlainAuth("", from, password, smtpHost)

	msg := []byte(fmt.Sprintf(`To: %s
Subject: 验证码邮件
MIME-Version: 1.0
Content-Type: text/html; charset="UTF-8"

<!DOCTYPE html>
<html>
<head>
  <style>
    body {
      font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen,
                   Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
      background-color: #f5f5f7;
      color: #1d1d1f;
      margin: 0;
      padding: 0;
    }
    .container {
      max-width: 480px;
      margin: 40px auto;
      background: white;
      border-radius: 12px;
      padding: 30px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.1);
      text-align: center;
    }
    h1 {
      font-weight: 600;
      font-size: 24px;
      margin-bottom: 10px;
    }
    p {
      font-size: 16px;
      line-height: 1.5;
      margin-top: 0;
      margin-bottom: 30px;
    }
    .code {
      display: inline-block;
      font-size: 32px;
      letter-spacing: 6px;
      background: #f0f0f5;
      padding: 15px 40px;
      border-radius: 10px;
      font-weight: 700;
      color: #007aff; /* 苹果蓝 */
      user-select: all;
    }
    footer {
      font-size: 12px;
      color: #888;
      margin-top: 40px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>邮箱验证码</h1>
    <p>您好，您的验证码如下，请在10分钟内使用：</p>
    <div class="code">%s</div>
    <footer>如果不是您本人操作，请忽略此邮件。</footer>
  </div>
</body>
</html>
`, toEmail, code))

	addr := smtpHost + ":" + smtpPort
	err := smtp.SendMail(addr, auth, from, []string{toEmail}, msg)
	if err != nil {
		return err
	}

	fmt.Printf("验证码 %s 已发送到 %s\n", code, toEmail)
	return nil
}

func SendEmailGmail(c *gin.Context) {
	email := c.PostForm("email")
	if email == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "邮箱不能为空"})
		return
	}
	// 检查 Redis 中是否已存在未过期的验证码，防止重复发送
	key := "verify_code:" + email
	exists, err := hiredis.RedisClient.Exists(hiredis.Ctx, key).Result()
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}
	if exists > 0 {
		c.JSON(http.StatusTooManyRequests, gin.H{"error": "验证码已发送，请稍后再试"})
		return
	}

	// 2. 判断邮箱是否已注册
	var user models.User
	err = models.DB.Where("email = ?", email).First(&user).Error
	if err != nil && !errors.Is(err, gorm.ErrRecordNotFound) {
		// 除了没找到，其他错误都算服务器错误
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}
	if err == nil {
		// 找到了，说明邮箱已注册
		c.JSON(http.StatusBadRequest, gin.H{"error": "邮箱已注册"})
		return
	}
	// err == gorm.ErrRecordNotFound 表示没注册，继续往下执行

	//3. 发送验证码
	code := GenerateCode()

	if err := SendVerificationCodeGmail(email, code); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "发送验证码失败"})
		return
	}

	// 保存验证码到 Redis，10分钟过期
	if err := hiredis.RedisClient.Set(hiredis.Ctx, key, code, 180*time.Second).Err(); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}

	c.JSON(http.StatusOK, gin.H{"message": "验证码已发送"})
}

func EmailGmailRegister(c *gin.Context) {
	email := c.PostForm("email")
	code := c.PostForm("code")
	if email == "" || code == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "邮箱和验证码不能为空"})
		return
	}

	key := "verify_code:" + email
	savedCode, err := hiredis.RedisClient.Get(hiredis.Ctx, key).Result()
	if err == redis.Nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "验证码不存在或已过期"})
		return
	} else if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}

	if savedCode != code {
		c.JSON(http.StatusBadRequest, gin.H{"error": "验证码错误"})
		return
	}

	// 判断邮箱是否已注册
	var user models.User
	err = models.DB.Where("email = ?", email).First(&user).Error
	if err != nil && !errors.Is(err, gorm.ErrRecordNotFound) {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}
	if err == nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": "邮箱已注册"})
		return
	}

	deviceID := c.GetHeader("X-Device-ID")
	ip := c.ClientIP()

	var newUser models.User
	err = models.DB.Transaction(func(tx *gorm.DB) error {
		emailCopy := email
		newUser = models.User{
			Email:    &emailCopy,
			Role:     1,
			IsActive: true,
		}

		if err := tx.Create(&newUser).Error; err != nil {
			return err
		}

		identity := models.Identity{
			UserID:       newUser.UserID,
			IdentityType: "email",
			Identifier:   email,
			IsVerified:   true,
		}
		if err := tx.Create(&identity).Error; err != nil {
			return err
		}

		var devicePtr *string
		if deviceID != "" {
			devicePtr = &deviceID
		}
		var ipPtr *string
		if ip != "" {
			ipPtr = &ip
		}
		risk := models.UserRisk{
			UserID:      newUser.UserID,
			DeviceID:    devicePtr,
			IPAddress:   ipPtr,
			RiskLevel:   0,
			RiskType:    "registration",
			Status:      "pending",
			TriggeredAt: time.Now(),
		}
		if err := tx.Create(&risk).Error; err != nil {
			return err
		}
		return nil
	})
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}

	// 生成访问和刷新令牌
	newAccessToken, err := generateAccessToken(email, newUser.UserID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "生成访问令牌失败"})
		return
	}
	newRefreshToken, err := generateRefreshToken(email, newUser.UserID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "生成刷新令牌失败"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"access_token":  newAccessToken,
		"refresh_token": newRefreshToken,
		"message":       "注册成功",
		"code":          200,
	})
}

func RefreshTokenHandler(c *gin.Context) {
	refreshToken := c.PostForm("refresh_token")
	if refreshToken == "" {
		c.JSON(http.StatusBadRequest, gin.H{"error": "刷新令牌不能为空"})
		return
	}

	token, err := jwt.Parse(refreshToken, func(token *jwt.Token) (interface{}, error) {
		// 确保签名方法符合预期
		if _, ok := token.Method.(*jwt.SigningMethodHMAC); !ok {
			return nil, fmt.Errorf("unexpected signing method")
		}
		return jwtSecret, nil
	})
	if err != nil || !token.Valid {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "刷新令牌无效，请重新登录"})
		return
	}

	claims, ok := token.Claims.(jwt.MapClaims)
	if !ok {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "刷新令牌无效"})
		return
	}

	identifier, ok := claims["identifier"].(string)
	if !ok || identifier == "" {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "刷新令牌无效"})
		return
	}

	userIDFloat, ok := claims["user_id"].(float64)
	if !ok {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "刷新令牌无效"})
		return
	}
	userID := uint(userIDFloat)

	var user models.User
	if err := models.DB.Where("user_id = ?", userID).First(&user).Error; err != nil {
		if errors.Is(err, gorm.ErrRecordNotFound) {
			c.JSON(http.StatusUnauthorized, gin.H{"error": "用户不存在"})
		} else {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		}
		return
	}
	// 你可以再判断用户是否激活等
	if !user.IsActive {
		c.JSON(http.StatusUnauthorized, gin.H{"error": "用户已被禁用"})
		return
	}

	newAccessToken, err := generateAccessToken(identifier, userID)
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "生成访问令牌失败"})
		return
	}

	c.JSON(http.StatusOK, gin.H{
		"access_token": newAccessToken,
	})
}

var jwtSecret = []byte("a3f9c1d4e5b67890f2a1b3c4d5e6f7890123456789abcdef0123456789abcdef") // 生产环境建议从配置或环境变量加载

func generateAccessToken(email string, userid uint) (string, error) {
	claims := jwt.MapClaims{
		"identifier": email,
		"user_id":    userid,
		"exp":        time.Now().Add(15 * time.Minute).Unix(), // 15分钟有效
		"iat":        time.Now().Unix(),
	}
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	return token.SignedString(jwtSecret)
}

func generateRefreshToken(email string, userid uint) (string, error) {
	claims := jwt.MapClaims{
		"identifier": email,
		"user_id":    userid,
		"exp":        time.Now().Add(7 * 24 * time.Hour).Unix(), // 7天有效
		"iat":        time.Now().Unix(),
	}
	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	return token.SignedString(jwtSecret)
}
