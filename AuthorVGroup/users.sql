-- 用户主表
CREATE TABLE IF NOT EXISTS users (
    user_id     SERIAL PRIMARY KEY,
    username    VARCHAR(50) UNIQUE,                       -- 用户昵称或登录名（可为空）
    email       VARCHAR(100) UNIQUE,                      -- 主邮箱（可为空）
    role        INTEGER NOT NULL DEFAULT 0,               -- 用户角色（按业务定义）
    is_active   BOOLEAN NOT NULL DEFAULT TRUE,            -- 是否激活
    created_at  TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at  TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    last_login_at TIMESTAMP WITH TIME ZONE                  -- 最近登录时间
    );

-- 自动更新 updated_at 字段触发器函数
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
   NEW.updated_at = CURRENT_TIMESTAMP;
RETURN NEW;
END;
$$ language 'plpgsql';

-- 触发器：更新 users.updated_at
CREATE TRIGGER trg_users_updated_at
    BEFORE UPDATE ON users
    FOR EACH ROW EXECUTE PROCEDURE update_updated_at_column();

-- 身份认证表，支持多种登录方式
CREATE TABLE IF NOT EXISTS identities (
    identity_id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    identity_type VARCHAR(20) NOT NULL,          -- 'email', 'google', 'wechat', 'phone', ...
    identifier VARCHAR(150) NOT NULL,             -- 唯一标识（如Google的sub）
    is_verified BOOLEAN NOT NULL DEFAULT FALSE,
    created_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    UNIQUE (identity_type, identifier)
    );

-- 触发器：更新 identities.updated_at
CREATE TRIGGER trg_identities_updated_at
    BEFORE UPDATE ON identities
    FOR EACH ROW EXECUTE PROCEDURE update_updated_at_column();

-- 用户风险表
CREATE TABLE IF NOT EXISTS user_risks (
    id              SERIAL PRIMARY KEY,
    user_id         INTEGER NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    device_id       VARCHAR(100),
    ip_address      VARCHAR(45),
    risk_level      INTEGER NOT NULL DEFAULT 0,           -- 风险等级：0~4
    risk_type       VARCHAR(50) NOT NULL,                 -- 风险类型：登录异常、多地登录等
    risk_source     VARCHAR(100),                         -- 风险来源，例如“登录服务”
    description     TEXT,                                 -- 风险描述详情
    triggered_at    TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,  -- 风险触发时间
    resolved_at     TIMESTAMP WITH TIME ZONE,             -- 风险解除时间
    status          VARCHAR(20) NOT NULL DEFAULT 'pending', -- 状态：pending/resolved/expired 等
    handler         VARCHAR(50),                          -- 处理人或系统服务名
    related_user_id INTEGER REFERENCES users(user_id),    -- 相关用户（如共享账号风险）
    note            TEXT,                                 -- 备注信息
    created_at      TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP,
    updated_at      TIMESTAMP WITH TIME ZONE DEFAULT CURRENT_TIMESTAMP
    );

-- 触发器：更新 user_risks.updated_at
CREATE TRIGGER trg_user_risks_updated_at
    BEFORE UPDATE ON user_risks
    FOR EACH ROW EXECUTE PROCEDURE update_updated_at_column();

-- 索引部分

-- users 表索引
CREATE UNIQUE INDEX IF NOT EXISTS idx_users_username ON users(username);
CREATE UNIQUE INDEX IF NOT EXISTS idx_users_email ON users(email);
CREATE INDEX IF NOT EXISTS idx_users_role ON users(role);
CREATE INDEX IF NOT EXISTS idx_users_is_active ON users(is_active);

-- identities 表索引
CREATE UNIQUE INDEX IF NOT EXISTS idx_identities_identity_type_identifier ON identities(identity_type, identifier);
CREATE INDEX IF NOT EXISTS idx_identities_user_id ON identities(user_id);

-- user_risks 表索引
CREATE INDEX IF NOT EXISTS idx_risks_user_id ON user_risks(user_id);
CREATE INDEX IF NOT EXISTS idx_risks_related_user_id ON user_risks(related_user_id);
CREATE INDEX IF NOT EXISTS idx_risks_risk_type ON user_risks(risk_type);
CREATE INDEX IF NOT EXISTS idx_risks_status ON user_risks(status);
CREATE INDEX IF NOT EXISTS idx_risks_device_id ON user_risks(device_id);
CREATE INDEX IF NOT EXISTS idx_risks_ip_address ON user_risks(ip_address);
CREATE INDEX IF NOT EXISTS idx_risks_user_id_status ON user_risks(user_id, status);
