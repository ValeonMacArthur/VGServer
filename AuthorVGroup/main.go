package main

import (
	"AuthorVGroup/hiredis"
	"AuthorVGroup/models"
	"AuthorVGroup/router"
	"fmt"
	"log"
	"os"

	"github.com/gin-gonic/gin"
)

func main() {
	models.InitDB()
	hiredis.InitRedis()

	// 1. 读取运行模式（默认 debug）
	ginMode := os.Getenv("GIN_MODE")
	if ginMode == "" {
		ginMode = gin.DebugMode // 或 gin.ReleaseMode
	}
	gin.SetMode(ginMode)
	// 4. 读取端口（默认 8080）
	port := os.Getenv("APP_PORT")
	if port == "" {
		port = "8080"
	}
	r := router.SetupRouter()
	addr := fmt.Sprintf(":%s", port)
	log.Printf("Starting server in %s mode on port %s", ginMode, port)
	if err := r.Run(addr); err != nil {
		log.Fatalf("Failed to start server: %v", err)
	}
}
