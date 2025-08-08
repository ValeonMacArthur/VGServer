```aiignore




+-------------+        +-------------+       +-------------+
|   Person    |◄──────►|   Reward    |◄─────►|   Video     |
+-------------+        +-------------+       +-------------+
| user_id     |        | reward_id   |       | video_id    |
| nickname    |        | user_id     |       | title       |
| level       |        | video_id    |       | play_url    |
| is_vip      |        | gift_type   |       | series_id   |
| coin_balance|        | gift_value  |       | coin_price  |
+-------------+        | created_at  |       | is_vip_only |
+-------------+       +-------------+
▲
│
│
+-------------+
|  Purchase   |
+-------------+
| purchase_id |
| user_id     |
| item_type   |
| item_id     |
| coin_amount |
| created_at  |
+-------------+

        +-------------+
        |   Series    |
        +-------------+
        | series_id   |
        | title       |
        | description |
        +-------------+
             ▲
             │
             │
       +-------------+
       |   Video     |
       +-------------+
```

