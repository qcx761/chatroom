// 存入
json j = {
    {"account", account},
    {"username", username},
    {"password", password}
};
auto stmt = conn->prepareStatement("INSERT INTO users (user_json) VALUES (?)");
stmt->setString(1, j.dump());
stmt->executeUpdate();




// 查找
auto stmt = conn->prepareStatement(
    "SELECT user_json FROM users WHERE JSON_EXTRACT(user_json, '$.account') = ?");
stmt->setString(1, account);
auto res = stmt->executeQuery();

if (res->next()) {
    // 已存在该 account
}

// 1. 连接Redis
Redis redis("tcp://127.0.0.1:6379");

// 2. 生成token
std::string token = generate_token();

// 3. 拼接Redis key（比如用账号做key，或者token做key）
std::string redis_key = "user:" + user_info["account"];

// 4. 构造一个json对象，包含账号、密码、用户名、token
json user_data = {
    {"account", user_info["account"]},
    {"password", user_info["password"]},   // 注意：实际生产不要明文存密码！
    {"username", user_info["username"]},
    {"token", token}
};

// 5. 存入Redis（字符串格式）
redis.set(redis_key, user_data.dump());

// 6. 设置过期时间，比如1小时（3600秒）
redis.expire(redis_key, 3600);
