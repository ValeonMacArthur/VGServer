package router

import (
	"github.com/gin-gonic/gin"
)

func SetupRouter() *gin.Engine {
	gin.SetMode(gin.ReleaseMode)
	r := gin.Default()

	//email
	r.POST("/send_code", SendEmailGmail)
	r.POST("/email_register", EmailGmailRegister)
	r.POST("/refresh_token", RefreshTokenHandler)

	return r
}
