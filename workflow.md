# Git 开发流程指南

## 分支策略

- `main`：主分支，保持稳定，部署用。
- `release-x.x.x`：发布分支，用于维护某个发布版本。
- `feature/xxx`：功能分支，用于开发新功能。

## 新功能开发流程

1. 从 `main` 拉取最新代码，创建功能分支：

    ```bash
    git checkout main
    git pull origin main
    git fetch --prune
    git checkout -b feature/your-feature-name
    ```

2. 在功能分支开发，提交并推送：

    ```bash
    git add .
    git commit -m "实现了 xxx 功能"
    git push origin feature/your-feature-name
    ```

3. 开发过程中，如需同步 `main` 最新代码到功能分支，执行：

    ```bash
    git checkout feature/your-feature-name
    git fetch origin
    git merge origin/main
    ```

   > 如果遇到冲突，解决后执行：

    ```bash
    git add <解决冲突的文件>
    git commit
    ```

   > 解决冲突后执行：

    ```bash
    git add <冲突文件>
    git rebase --continue
    ```

4. 功能完成后，合并回 `main` 分支：

    ```bash
    git checkout main
    git pull origin main
    git merge feature/your-feature-name
    git push origin main
    ```

## 发布新版本流程

1. 从 `main` 创建新的发布分支：

    ```bash
    git checkout main
    git pull origin main
    git checkout -b release-x.x.x
    git push origin release-x.x.x
    ```

2. 发布分支用于修复和维护对应版本，修复后可合并回 `main`：

    ```bash
    git checkout release-x.x.x
    git merge main
    git push origin release-x.x.x
    ```

---

此流程保证 `main` 保持稳定，功能开发有独立分支，发布分支便于版本管理和修复。

---

