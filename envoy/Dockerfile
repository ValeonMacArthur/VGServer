# envoy/Dockerfile
# 部署的时候自己看着修改
FROM envoyproxy/envoy:v1.30.0
COPY envoy.yaml /etc/envoy/envoy.yaml
CMD ["envoy", "-c", "/etc/envoy/envoy.yaml", "--log-level", "info"]