# 破坏者(bugstd.com) 项目概览

## 项目简介

VGServer 是一个分布式系统，采用 Go 和 C++20 混合架构设计：

- **Go服务**：负责用户登录、注册、权限认证（JWT生成与验证）、Apple ID授权登录、短信验证码等安全相关功能。对外暴露 API 接口。
- **C++服务**：业务逻辑处理，C++20编写，性能优先。只负责解析JWT，判断是否有效，结合数据库做业务授权判断，专注业务实现。
- **架构设计**：
  - 域名分流：
    - `https://auth.vgstudio.com` - 认证和用户管理服务，Go实现
    - `https://api.vgstudio.com` - 业务服务，C++实现
    - `https://admin.vgstudio.com` - 后台管理，go实现
    - `https://static.vgstudio.com` - 静态html服务
    - `https://oss-shenzhen.vgstudio.com` - 静态html服务
  - 数据存储：
    - PostgreSQL：用户、身份、业务数据持久化
    - Redis：缓存验证码、会话管理等
  - 项目部署Docker：
    通过docker管理实例
## 项目目录架构


```aiignore

                  ┌───────────────────────────┐
                  │      External Client      │
                  └─────────────┬─────────────┘
                                │ https://vgserver.example.com
                                ▼
                  ┌───────────────────────────┐
                  │     Istio Gateway         │
                  │  (Ingress Gateway)        │
                  └─────────────┬─────────────┘
                                │ 匹配 hosts & port
                                ▼
                  ┌───────────────────────────┐
                  │    VirtualService         │
                  │  路由规则 + 重试/超时        │
                  └─────────────┬─────────────┘
                                │ route → vgserver-svc
                                ▼
                  ┌───────────────────────────┐
                  │   k8s Service vgserver-svc│
                  │ ClusterIP / LoadBalancer  │
                  │ Selector: app=vgserver    │
                  └─────────────┬─────────────┘
                                │ 负载均衡到 Pod 副本
                                ▼
         ┌───────────────k8s Pod #1 ────────────────┐
         │ Deployment: vgserver-deploy           │
         │ ┌───────────────┐  ┌───────────────┐ │
         │ │ Envoy Sidecar │  │ VGServer App │ │
         │ └───────────────┘  └───────────────┘ │
         └───────────────┬───────────────┘
                         │
         ┌─────────────── Pod #2 ────────────────┐
         │ ┌───────────────┐  ┌───────────────┐ │
         │ │ Envoy Sidecar │  │ VGServer App │ │
         │ └───────────────┘  └───────────────┘ │
         └───────────────┬───────────────┘
                         │
         ┌─────────────── Pod #3 ────────────────┐
         │ ┌───────────────┐  ┌───────────────┐ │
         │ │ Envoy Sidecar │  │ VGServer App │ │
         │ └───────────────┘  └───────────────┘ │
         └──────────────────────────────────────┘

```

