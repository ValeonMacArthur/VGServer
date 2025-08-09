package models

import (
	"time"
)

// User 用户主表
type User struct {
	UserID      uint       `gorm:"column:user_id;primaryKey;autoIncrement" json:"user_id"`
	Username    *string    `gorm:"column:username;unique" json:"username,omitempty"` // 可为空，用指针
	Email       *string    `gorm:"column:email;unique" json:"email,omitempty"`       // 可为空，用指针
	Role        int        `gorm:"column:role;not null;default:0" json:"role"`
	IsActive    bool       `gorm:"column:is_active;not null;default:true" json:"is_active"`
	CreatedAt   time.Time  `gorm:"column:created_at;autoCreateTime" json:"created_at"`
	UpdatedAt   time.Time  `gorm:"column:updated_at;autoUpdateTime" json:"updated_at"`
	LastLoginAt *time.Time `gorm:"column:last_login_at" json:"last_login_at,omitempty"` // 可为空，用指针
}

// TableName 显式指定表名
func (User) TableName() string {
	return "users"
}

// Identity 身份认证表
type Identity struct {
	IdentityID   uint      `gorm:"column:identity_id;primaryKey;autoIncrement" json:"identity_id"`
	UserID       uint      `gorm:"column:user_id;not null" json:"user_id"`
	IdentityType string    `gorm:"column:identity_type;size:20;not null" json:"identity_type"`
	Identifier   string    `gorm:"column:identifier;size:150;not null" json:"identifier"`
	IsVerified   bool      `gorm:"column:is_verified;not null;default:false" json:"is_verified"`
	CreatedAt    time.Time `gorm:"column:created_at;autoCreateTime" json:"created_at"`
	UpdatedAt    time.Time `gorm:"column:updated_at;autoUpdateTime" json:"updated_at"`
}

// TableName 显式指定表名
func (Identity) TableName() string {
	return "identities"
}

// UserRisk 用户风险表
type UserRisk struct {
	ID            uint       `gorm:"column:id;primaryKey;autoIncrement" json:"id"`
	UserID        uint       `gorm:"column:user_id;not null" json:"user_id"`
	DeviceID      *string    `gorm:"column:device_id;size:100" json:"device_id,omitempty"`
	IPAddress     *string    `gorm:"column:ip_address;size:45" json:"ip_address,omitempty"`
	RiskLevel     int        `gorm:"column:risk_level;not null;default:0" json:"risk_level"`
	RiskType      string     `gorm:"column:risk_type;size:50;not null" json:"risk_type"`
	RiskSource    *string    `gorm:"column:risk_source;size:100" json:"risk_source,omitempty"`
	Description   *string    `gorm:"column:description" json:"description,omitempty"`
	TriggeredAt   time.Time  `gorm:"column:triggered_at;autoCreateTime" json:"triggered_at"`
	ResolvedAt    *time.Time `gorm:"column:resolved_at" json:"resolved_at,omitempty"`
	Status        string     `gorm:"column:status;size:20;not null;default:'pending'" json:"status"`
	Handler       *string    `gorm:"column:handler;size:50" json:"handler,omitempty"`
	RelatedUserID *uint      `gorm:"column:related_user_id" json:"related_user_id,omitempty"`
	Note          *string    `gorm:"column:note" json:"note,omitempty"`
	CreatedAt     time.Time  `gorm:"column:created_at;autoCreateTime" json:"created_at"`
	UpdatedAt     time.Time  `gorm:"column:updated_at;autoUpdateTime" json:"updated_at"`
}

// TableName 显式指定表名
func (UserRisk) TableName() string {
	return "user_risks"
}
