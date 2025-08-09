package hiredis

import (
	"context"
	"log"

	"github.com/go-redis/redis/v8"
)

var (
	Ctx         = context.Background()
	RedisClient *redis.Client
)

func InitRedis() {
	RedisClient = redis.NewClient(&redis.Options{
		Addr: "localhost:6379",
		DB:   0,
	})

	if err := RedisClient.Ping(Ctx).Err(); err != nil {
		log.Fatalf("连接Redis失败: %v", err)
	}
}
