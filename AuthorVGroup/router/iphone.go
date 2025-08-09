package router

import (
	"AuthorVGroup/hiredis"
	"fmt"
	"net/http"
	"time"

	"github.com/gin-gonic/gin"
	"github.com/go-redis/redis/v8"
	"github.com/nyaruka/phonenumbers"

	"github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common"
	"github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/common/profile"
	sms "github.com/tencentcloud/tencentcloud-sdk-go/tencentcloud/sms/v20210111"

	plivo "github.com/plivo/plivo-go"
)

func isValidPhone(phone, region string) bool {
	num, err := phonenumbers.Parse(phone, region)
	if err != nil {
		return false
	}
	return phonenumbers.IsValidNumber(num)
}

func SendSmsCode(c *gin.Context) {
	phone := c.PostForm("phone")
	if !isValidPhone(phone, "CN") {
		c.JSON(http.StatusBadRequest, gin.H{"error": "手机号格式错误"})
		return
	}

	// 限流逻辑，检查Redis是否频繁请求（略）

	code := GenerateCode() // 6位数字
	key := "verify_code:" + phone
	err := hiredis.RedisClient.Set(hiredis.Ctx, key, code, 10*time.Minute).Err()
	if err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "服务器错误"})
		return
	}

	// 调用短信服务发送code，伪函数
	if err := sendSMSWithTencentCloud(phone, fmt.Sprintf("你的验证码是: %s", code)); err != nil {
		c.JSON(http.StatusInternalServerError, gin.H{"error": "发送短信失败"})
		return
	}

	c.JSON(http.StatusOK, gin.H{"message": "验证码已发送"})
}

func VerifySmsCode(c *gin.Context) {
	phone := c.PostForm("phone")
	code := c.PostForm("code")

	savedCode, err := hiredis.RedisClient.Get(hiredis.Ctx, "verify_code:"+phone).Result()
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

	// 验证成功，删掉验证码避免重用
	hiredis.RedisClient.Del(hiredis.Ctx, "verify_code:"+phone)

	// 后续注册或登录操作
	c.JSON(http.StatusOK, gin.H{"message": "验证成功"})
}

func sendSMSWithTencentCloud(to string, code string) error {
	credential := common.NewCredential(
		"你的SecretId",
		"你的SecretKey",
	)
	cpf := profile.NewClientProfile()
	client, _ := sms.NewClient(credential, "ap-guangzhou", cpf)

	request := sms.NewSendSmsRequest()

	request.SmsSdkAppId = common.StringPtr("你的SmsSdkAppId")  // 短信SdkAppId
	request.SignName = common.StringPtr("你的签名")              // 短信签名
	request.TemplateId = common.StringPtr("你的模板ID")          // 模板ID
	request.PhoneNumberSet = common.StringPtrs([]string{to}) // 接收手机号，格式 +国家码手机号，例如 +8613711112222

	// 模板参数，比如验证码
	request.TemplateParamSet = common.StringPtrs([]string{code})

	response, err := client.SendSms(request)
	if err != nil {
		return err
	}

	fmt.Printf("Send SMS Response: %s\n", response.ToJsonString())
	return nil
}

func sendSMSWithPlivo(src, dst, text string) error {
	client, err := plivo.NewClient("YOUR_AUTH_ID", "YOUR_AUTH_TOKEN", &plivo.ClientOptions{})
	if err != nil {
		return err
	}

	params := plivo.MessageCreateParams{
		Src:  src,  // 发送方号码，必须是你Plivo账号里可用的号码
		Dst:  dst,  // 接收方号码，格式必须是 E.164 格式，例如 +8613711112222
		Text: text, // 短信内容
	}

	response, err := client.Messages.Create(params)
	if err != nil {
		return err
	}

	fmt.Printf("Message UUID: %s\n", response.MessageUUID)
	return nil
}
