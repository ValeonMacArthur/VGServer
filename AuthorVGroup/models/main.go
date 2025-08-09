package models

import (
	"fmt"
	"log"
	"os"
	"time"

	_ "github.com/lib/pq"
	"gorm.io/driver/postgres"
	"gorm.io/gorm"
)

func CreateTable(db *gorm.DB) {
	sqlBytes, err := os.ReadFile("./users.sql")
	if err != nil {
		log.Fatalf("Failed to read SQL file: %v", err)
	}

	if err := db.Exec(string(sqlBytes)).Error; err != nil {
		log.Fatalf("Failed to execute schema: %v", err)
	}
}

var DB *gorm.DB // 全局 GORM 对象

func InitDB() {
	dsn := "host=localhost user=postgres password=Rg9vTzXr82bqLpNfU3nF dbname=mydatabase port=5432 sslmode=disable"

	db, err := gorm.Open(postgres.Open(dsn), &gorm.Config{})
	if err != nil {
		log.Fatalf("failed to connect db: %v", err)
	}
	sqlDB, err := db.DB()
	if err != nil {
		log.Fatalf("failed to get generic db object: %v", err)
	}

	if err := sqlDB.Ping(); err != nil {
		log.Fatalf("failed to ping db: %v", err)
	}

	// 配置连接池
	sqlDB.SetMaxOpenConns(50)                  // 最大连接数
	sqlDB.SetMaxIdleConns(10)                  // 最大空闲连接数
	sqlDB.SetConnMaxLifetime(time.Hour)        // 单个连接最长存活时间
	sqlDB.SetConnMaxIdleTime(10 * time.Minute) // 空闲连接最长时间

	DB = db
	fmt.Println("✅ Database initialized and connection pool configured")
}
