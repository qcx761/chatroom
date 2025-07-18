#include "/home/mcy-mcy/文档/chatroom/include/inetsockets_tcp.hpp"
#include "/home/mcy-mcy/文档/chatroom/cli/menu.hpp"
#include "/home/mcy-mcy/文档/chatroom/define/define.hpp"
#include "/home/mcy-mcy/文档/chatroom/include/Threadpool.hpp"
#include <sys/epoll.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "User.hpp"
#include <nlohmann/json.hpp>
#include <sys/stat.h>
#include <sys/sendfile.h>

using json = nlohmann::json;

typedef struct file_args
{
    bool retr_flag;
    bool stor_flag;
    bool run_flag;
    bool end_flag;
    bool get_num_flag;
    string filename;
    int data_num;
    int FTP_data_cfd;
    int FTP_ctrl_cfd;
    int chat_server_cfd;
    pthread_cond_t *cond;
    pthread_mutex_t *lock;
    unordered_map<int, file_args *> *datafd_to_file_args;
} file_args;

typedef struct recv_args
{
    int cfd;
    int epfd;
    int FTP_ctrl_cfd;
    pool *file_pool;
    string client_num;
    string *file_name;
    string *username;
    string *fog_username;
    bool *end_flag;
    bool *end_start_flag;
    bool *fog_que_flag;
    bool *pri_chat_flag;
    bool *chat_name_flag;
    bool *rl_display_flag;
    bool *add_friend_req_flag;
    bool *end_chat_flag;
    bool *FTP_data_stor_flag;
    bool *FTP_data_retr_flag;
    vector<string> *add_friend_fri_user;
    vector<string> *add_friend_requests;
    pthread_cond_t *recv_cond;
    pthread_mutex_t *recv_lock;
    unordered_map<int, file_args *> *datafd_to_file_args;
} recv_args;

class client : public menu
{

public:
    client(int in_cfd, int FTP_ctrl_cfd, string client_num) : 
        end_start_flag(false), end_chat_flag(true), end_flag(false), handle_login_flag(true),
        fog_que_flag(false), add_friend_req_flag(false), chat_name_flag(false), pri_chat_flag(false),
        rl_display_flag(false), chat_choice(0), start_choice(0),
        cfd(in_cfd), FTP_ctrl_cfd(FTP_ctrl_cfd), client_num(client_num),
        FTP_data_stor_flag(false), FTP_data_retr_flag(false)
    {

        epfd = epoll_create(EPSIZE);
        ev.data.fd = cfd;
        ev.events = EPOLLET | EPOLLRDHUP | EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_ADD, cfd, &ev);
        ev.data.fd = FTP_ctrl_cfd;
        ev.events = EPOLLET | EPOLLRDHUP | EPOLLIN;
        epoll_ctl(epfd, EPOLL_CTL_ADD, FTP_ctrl_cfd, &ev);

        file_pool = new pool(CLIENT_FILE_NUM);
        args = new recv_args;
        args->cfd = cfd;
        args->epfd = epfd;
        args->FTP_ctrl_cfd = FTP_ctrl_cfd;
        args->file_pool = file_pool;
        args->client_num = client_num;
        args->file_name = &file_name;
        args->end_flag = &end_flag;
        args->username = &username;
        args->fog_username = &fog_username;
        args->fog_que_flag = &fog_que_flag;
        args->pri_chat_flag = &pri_chat_flag;
        args->chat_name_flag = &chat_name_flag;
        args->end_chat_flag = &end_chat_flag;
        args->end_start_flag = &end_start_flag;
        args->rl_display_flag = &rl_display_flag;
        args->add_friend_req_flag = &add_friend_req_flag;
        args->add_friend_requests = &add_friend_requests;
        args->add_friend_fri_user = &add_friend_fri_user;
        args->FTP_data_retr_flag = &FTP_data_retr_flag;
        args->FTP_data_stor_flag = &FTP_data_stor_flag;
        args->datafd_to_file_args = &datafd_to_file_args;

        pthread_cond_init(&recv_cond, nullptr);
        pthread_mutex_init(&recv_lock, nullptr);
        args->recv_cond = &recv_cond;
        args->recv_lock = &recv_lock;
        pthread_create(&recv_pthread, nullptr, recv_thread, args);
    }

    void start()
    {

        while (!end_flag)
        {

            while (!end_start_flag)
            {

                this->start_show();

                if (!(cin >> start_choice))
                {
                    cout << "请输入数字选项..." << endl;
                    cin.clear();
                    wait_user_continue();
                    system("clear");
                    continue;
                }

                if (!end_start_flag)
                {
                    if (start_choice == LOGIN)
                    {
                        system("clear");
                        json *login = new json;
                        handle_login(login);
                        sendjson(*login, cfd);
                        delete login;
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        if (fog_que_flag && !end_flag)
                        {
                            cout << "请输入答案" << endl;
                            char *in_ans = new char[64];
                            cin >> in_ans;
                            json send_json = {
                                {"request", CHECK_ANS},
                                {"username", fog_username},
                                {"answer", in_ans}};
                            sendjson(send_json, cfd);
                            handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                            wait_user_continue();
                        }
                        else
                            wait_user_continue();
                    }
                    else if (start_choice == EXIT)
                    {
                        system("clear");
                        cout << "感谢使用聊天室，再见!" << endl;
                        end_start_flag = true;
                        end_flag = true;
                        sleep(1);
                    }
                    else if (start_choice == SIGNIN)
                    {
                        system("clear");
                        json *signin = new json;
                        handle_signin(signin);
                        sendjson(*signin, cfd);
                        delete signin;
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                    }
                    else if (start_choice == BREAK)
                    {
                        system("clear");
                        json *json_break = new json;
                        ;
                        handle_break(json_break);
                        sendjson(*json_break, cfd);
                        delete json_break;
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                    }
                    else
                    {
                        cout << "请输入正确的选项..." << endl;
                        wait_user_continue();
                        system("clear");
                        continue;
                    }
                }

                system("clear");
            }

            while (!end_chat_flag)
            {

                if (handle_login_flag && !end_chat_flag)
                {
                    handle_offline_login(cfd, username);
                    handle_success_login(cfd, username);
                    handle_login_flag = false;
                }

                this->chat_show();
                if (!(cin >> chat_choice))
                {
                    cout << "请输入数字选项..." << endl;
                    cin.clear();
                    wait_user_continue();
                    system("clear");
                    continue;
                }

                if (!end_chat_flag)
                {
                    switch (chat_choice)
                    {
                    case 1:
                    {
                        system("clear");
                        json logout = {
                            {"request", LOGOUT},
                            {"username", username}};
                        sendjson(logout, cfd);
                        end_chat_flag = true;
                        end_start_flag = false;
                        handle_login_flag = true;
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                        break;
                    }
                    case 2:
                    {
                        system("clear");
                        json *chat_name = new json;
                        rl_display_flag = true;
                        handle_chat_name(chat_name, username);
                        rl_display_flag = false;
                        string fri_user = (*chat_name)["fri_user"];
                        sendjson(*chat_name, cfd);
                        delete chat_name;
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        if (chat_name_flag && !end_flag)
                        {
                            json offline_pri;
                            handle_history_pri(&offline_pri, username);
                            sendjson(offline_pri, cfd);
                            handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                            if (pri_chat_flag && !end_flag)
                            {
                                rl_display_flag = true;
                                handle_pri_chat(username, fri_user, cfd, FTP_ctrl_cfd, &end_flag,
                                                &FTP_data_stor_flag, &pri_chat_flag, client_num,
                                                &recv_cond, &recv_lock, &file_name);
                                rl_display_flag = false;
                                pri_chat_flag = false;
                            }
                            cout << "=============================================" << endl;
                            wait_user_continue();
                            chat_name_flag = false;
                        }
                        else
                            wait_user_continue();
                        break;
                    }
                    case 3:
                    {
                        system("clear");
                        json *add_friend = new json;
                        handle_add_friend(add_friend, username);
                        sendjson(*add_friend, cfd);
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                        break;
                    }
                    case 4:
                    {
                        system("clear");
                        json *get_fri_req = new json;
                        handle_get_friend_req(get_fri_req, username);
                        sendjson(*get_fri_req, cfd);
                        delete (get_fri_req);
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        if (add_friend_req_flag && !end_flag)
                        {
                            bool add_flag = true;
                            vector<string> commit;
                            vector<string> refuse;
                            while (add_flag)
                            {
                                int num;
                                char chk;
                                cout << "请输入需要处理的好友申请的选项(输入-1退出交互): " << endl;
                                cin >> num;
                                if (num == 0)
                                {
                                    add_flag = false;
                                    break;
                                }
                                if ((num < 1 || num > add_friend_requests.size()) && num != -1)
                                {
                                    cout << "编号超出范围，请重新输入。" << endl;
                                    continue;
                                }
                                if (num == -1)
                                {
                                    add_flag = false;
                                    break;
                                }
                                cout << "请输入是否同意(y/n)" << endl;
                                cin >> chk;
                                if (chk == 'y')
                                {
                                    commit.push_back(add_friend_fri_user[num - 1]);
                                }
                                else if (chk == 'n')
                                {
                                    refuse.push_back(add_friend_fri_user[num - 1]);
                                }
                                else
                                {
                                    cout << "请勿输入无关选项..." << endl;
                                }
                            }
                            if (commit.size() == 0 && refuse.size() == 0)
                            {
                                cout << "未处理好友关系..." << endl;
                                sleep(1);
                                system("clear");
                                continue;
                            }
                            json send_json = {
                                {"request", DEAL_FRI_REQ},
                                {"commit", commit},
                                {"refuse", refuse},
                                {"username", username}};
                            sendjson(send_json, cfd);
                            handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                            wait_user_continue();
                        }
                        else
                            wait_user_continue();
                        break;
                    }
                    case 6:
                    {
                        system("clear");
                        handle_black(username, cfd, &rl_display_flag);
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                        break;
                    }
                    case 7:
                    {
                        system("clear");
                        handle_check_friend(username, cfd);
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                        break;
                    }
                    case 8:
                    {
                        system("clear");
                        rl_display_flag = true;
                        handle_delete_friend(username, cfd);
                        rl_display_flag = false;
                        handle_pthread_wait(end_flag, &recv_cond, &recv_lock);
                        wait_user_continue();
                        break;
                    }
                    default:
                    {
                        cout << "请输入正确的选项..." << endl;
                        wait_user_continue();
                        system("clear");
                        continue;
                    }
                    }

                    system("clear");
                }
            }
        }

        return;
    }

private:
    int cfd;
    int epfd;
    int FTP_ctrl_cfd;
    pool *file_pool;
    string file_name;
    bool FTP_data_stor_flag;
    bool FTP_data_retr_flag;
    bool end_flag;
    bool fog_que_flag;
    bool end_chat_flag;
    bool chat_name_flag;
    bool end_start_flag;
    bool pri_chat_flag;
    bool rl_display_flag;
    bool handle_login_flag;
    bool add_friend_req_flag;
    string client_num;
    string username;
    string fog_username;
    recv_args *args;
    int start_choice;
    int chat_choice;
    struct epoll_event ev;
    vector<string> add_friend_fri_user;
    vector<string> add_friend_requests;
    pthread_t recv_pthread;
    pthread_cond_t recv_cond;
    pthread_mutex_t recv_lock;
    unordered_map<int, file_args *> datafd_to_file_args;

    static file_args *find_data_num_pair(int data_num, unordered_map<int, file_args *> *datafd_to_file_args)
    {
        file_args *file_pair = nullptr;
        auto it = datafd_to_file_args->begin();
        for (; it != datafd_to_file_args->end(); it++)
        {
            if (it->second->data_num == data_num)
            {
                file_pair = it->second;
                break;
            }
        }
        return file_pair;
    }

    static void *file_pool_func(void *args)
    {
        file_args *new_args = (file_args *)args;
        pthread_cond_wait(new_args->cond, new_args->lock);
        if (new_args->run_flag)
        {
            pthread_cond_wait(new_args->cond, new_args->lock);
            if (new_args->retr_flag)
            {

                pthread_cond_wait(new_args->cond, new_args->lock);
                if (new_args->end_flag)
                {
                    close(new_args->FTP_data_cfd);
                }
                return nullptr;
            }
            if (new_args->stor_flag)
            {
                struct stat file_stat;
                ssize_t file_size;
                ssize_t send_size;
                off_t off_set = 0;

                int file_fd = open(new_args->filename.c_str(), O_RDONLY, 0754);
                stat(new_args->filename.c_str(), &file_stat);
                file_size = file_stat.st_size;
                send_size = file_size;

                if (file_size > CHUNK_SIZE)
                {
                    while (send_size > 0)
                    {
                        ssize_t hav_send = sendfile(new_args->FTP_data_cfd, file_fd, &off_set, CHUNK_SIZE);
                        send_size -= hav_send;
                    }
                }
                else
                    sendfile(new_args->FTP_data_cfd, file_fd, &off_set, CHUNK_SIZE);

                pthread_cond_wait(new_args->cond, new_args->lock);
                if (new_args->end_flag)
                {
                    delete (*new_args->datafd_to_file_args)[new_args->FTP_data_cfd];
                    (*new_args->datafd_to_file_args).erase(new_args->FTP_data_cfd);
                    close(new_args->FTP_data_cfd);
                }
                return nullptr;
            }
        }
        return nullptr;
    }

    static void *recv_thread(void *args)
    {

        string buffer;
        char recvbuf[MAXBUF] = {0};
        recv_args *new_args = (recv_args *)args;
        struct epoll_event evlist[1];

        while (true && !(*new_args->end_flag))
        {

            int num = epoll_wait(new_args->epfd, evlist, 1, -1);

            for (int i = 0; i < num; i++)
            {
                int chk_fd = evlist[i].data.fd;
                if (evlist[i].events & (EPOLLHUP | EPOLLERR | EPOLLRDHUP))
                {
                    if ((*new_args->datafd_to_file_args).count(chk_fd) == 0)
                    {
                        cout << "服务器已关闭，当前模块交互结束将退出程序...." << endl;
                        *new_args->end_flag = true;
                        *new_args->end_chat_flag = true;
                        *new_args->end_start_flag = true;
                        pthread_cond_signal(new_args->recv_cond);
                        return nullptr;
                    }
                    else
                    {
                        cout << "FTP datafd 关闭: " << chk_fd << endl;
                        close(chk_fd);
                        epoll_ctl(new_args->epfd, EPOLL_CTL_DEL, chk_fd, NULL);
                        delete (*new_args->datafd_to_file_args)[chk_fd];
                        (*new_args->datafd_to_file_args).erase(chk_fd);
                        continue;
                    }
                }
                else if (evlist[i].events & EPOLLIN)
                {
                    if ((*new_args->datafd_to_file_args).count(chk_fd) > 0)
                    {
                        file_args *data_new_args = (*new_args->datafd_to_file_args)[chk_fd];
                        if (data_new_args->get_num_flag)
                        {
                            char in_num[10];
                            recv(evlist[i].data.fd, in_num, sizeof(in_num) - 1, 0);
                            data_new_args->data_num = atoi(in_num);
                            cout << "get num: [" << in_num << "]" << endl;
                            if (data_new_args->stor_flag)
                            {
                                char stor_cmd[NOMSIZE];
                                sprintf(stor_cmd, "%s %s+%s", "STOR", new_args->file_name->c_str(), in_num);
                                send(new_args->FTP_ctrl_cfd, stor_cmd, strlen(stor_cmd) + 1, 0);
                                cout << "send-2-stor: " << stor_cmd << endl;
                                new_args->file_pool->addtask(file_pool_func, data_new_args);
                            }
                            else if (data_new_args->retr_flag)
                            {
                                char retr_cmd[NOMSIZE];
                                sprintf(retr_cmd, "%s %s+%s", "RETR", new_args->file_name->c_str(), in_num);
                                send(new_args->FTP_ctrl_cfd, retr_cmd, strlen(retr_cmd) + 1, 0);
                                cout << "send-2-retr: " << retr_cmd << endl;
                                new_args->file_pool->addtask(file_pool_func, data_new_args);
                            }
                            data_new_args->get_num_flag = false;
                        }
                        else
                            new_args->file_pool->addtask(file_pool_func, data_new_args);
                        continue;
                    }

                    ssize_t n;
                    while ((n = recv(evlist[i].data.fd, recvbuf, MAXBUF, MSG_DONTWAIT)) > 0)
                    {
                        buffer.append(recvbuf, n);
                    }
                    if (n == -1)
                    {
                        if (errno != EAGAIN && errno != EWOULDBLOCK)
                        {
                            perror("recv");
                            return nullptr;
                        }
                    }
                    while (buffer.size() >= 4)
                    {

                        uint32_t net_len;
                        memcpy(&net_len, buffer.data(), 4);
                        uint32_t msg_len = ntohl(net_len);
                        if (msg_len == 0 || msg_len > MAX_REASONABLE_SIZE)
                        {
                            cerr << "异常消息长度: " << msg_len << endl;
                            buffer.clear();
                            break;
                        }
                        if (buffer.size() < 4 + msg_len)
                            break;
                        string json_str = buffer.substr(4, msg_len);
                        buffer.erase(0, 4 + msg_len);

                        json recvjson;
                        try
                        {
                            recvjson = json::parse(json_str);
                        }
                        catch (...)
                        {
                            cerr << "JSON parse error\n";
                            continue;
                        }
                        if (recvjson["sort"] == REFLACT)
                        {
                            if (recvjson["request"] == SIGNIN)
                            {
                                cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == LOGIN)
                            {
                                if (recvjson["login_flag"])
                                {
                                    *new_args->end_start_flag = true;
                                    *new_args->end_chat_flag = false;
                                    *new_args->username = recvjson["username"];
                                }
                                cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == FORGET_PASSWORD)
                            {
                                if (recvjson["que_flag"])
                                {
                                    *new_args->fog_que_flag = true;
                                    *new_args->fog_username = recvjson["username"];
                                    cout << "密保问题: " << recvjson["reflact"] << endl;
                                }
                                else
                                    cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == CHECK_ANS)
                            {
                                if (recvjson["ans_flag"])
                                {
                                    *new_args->end_start_flag = true;
                                    *new_args->end_chat_flag = false;
                                    *new_args->username = recvjson["username"];
                                }
                                cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == LOGOUT)
                            {
                                cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == BREAK)
                            {
                                cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == ADD_FRIEND)
                            {
                                cout << recvjson["reflact"] << endl;
                            }
                            else if (recvjson["request"] == GET_FRIEND_REQ)
                            {
                                if (recvjson["do_flag"] == false)
                                {
                                    cout << recvjson["reflact"] << endl;
                                }
                                else
                                {
                                    *new_args->add_friend_req_flag = true;
                                    *new_args->add_friend_fri_user = recvjson["fri_user"];
                                    *new_args->add_friend_requests = recvjson["reflact"];
                                    cout << "你有 " << (*new_args->add_friend_requests).size()
                                         << " 条好友申请：" << endl;
                                    for (size_t i = 0; i < (*new_args->add_friend_requests).size(); ++i)
                                    {
                                        cout << i + 1 << ". " << (*new_args->add_friend_requests)[i] << endl;
                                    }
                                }
                            }
                            else if (recvjson["request"] == DEAL_FRI_REQ)
                            {
                                string reflact = recvjson["reflact"];
                                cout << reflact << endl;
                            }
                            else if (recvjson["request"] == CHAT_NAME)
                            {
                                string reflact = recvjson["reflact"];
                                if (recvjson["chat_flag"] == false)
                                    cout << reflact << endl;
                                else
                                {
                                    cout << reflact << endl;
                                    cout << "=============================================" << endl;
                                    *new_args->chat_name_flag = true;
                                }
                            }
                            else if (recvjson["request"] == GET_HISTORY_PRI)
                            {
                                if (recvjson["ht_flag"] == false)
                                {
                                    string reflact = recvjson["reflact"];
                                    cout << reflact << '\n'
                                         << endl;
                                }
                                else
                                {
                                    json history = recvjson["reflact"];
                                    for (auto &msg : history)
                                    {
                                        string sender = msg["sender"];
                                        string content = msg["content"];
                                        string timestamp = msg["timestamp"];
                                        cout << "[" << timestamp << "] " << sender << ": " << content << endl;
                                    }
                                    cout << '\n';
                                }
                                *new_args->pri_chat_flag = true;
                            }
                            else if (recvjson["request"] == ADD_BLACKLIST)
                            {
                                string reflact = recvjson["reflact"];
                                cout << reflact << endl;
                            }
                            else if (recvjson["request"] == REMOVE_BLACKLIST)
                            {
                                string reflact = recvjson["reflact"];
                                cout << reflact << endl;
                            }
                            else if (recvjson["request"] == CHECK_FRIEND)
                            {
                                json friends = recvjson["friends"];
                                cout << "好友列表如下: " << endl;
                                for (size_t i = 0; i + 1 < friends.size(); i += 2)
                                {
                                    string name = friends[i];
                                    string status = friends[i + 1];
                                    cout << "好友" << (i + 1) / 2 + 1 << ": " << name << " 状态：" << status << endl;
                                }
                            }
                            else if (recvjson["request"] == DELETE_FRIEND)
                            {
                                string reflact = recvjson["reflact"];
                                cout << reflact << endl;
                            }
                            else if (recvjson["request"] == LIST_CMD)
                            {
                                string reflact = recvjson["reflact"];
                                cout << reflact << endl;
                            }
                            else if (recvjson["request"] == PASV_CMD)
                            {
                                string reflact = recvjson["reflact"];
                                cout << reflact << endl;
                                if (reflact.size() == 0)
                                {
                                    cout << "227 reflact error" << endl;
                                    continue;
                                }

                                int datafd = handle_pasv(reflact);
                                if (datafd == -1)
                                {
                                    cout << "pasv connect error" << endl;
                                    continue;
                                }
                                int flags = fcntl(datafd, F_GETFL, 0);
                                fcntl(datafd, F_SETFL, flags | O_NONBLOCK);

                                file_args *pool_file_args = new (file_args);
                                pool_file_args->FTP_data_cfd = datafd;
                                pool_file_args->FTP_ctrl_cfd = new_args->FTP_ctrl_cfd;
                                pool_file_args->chat_server_cfd = new_args->cfd;
                                pthread_cond_t *cond = new pthread_cond_t;
                                pthread_mutex_t *lock = new pthread_mutex_t;
                                pthread_cond_init(cond, nullptr);
                                pthread_mutex_init(lock, nullptr);
                                pool_file_args->cond = cond;
                                pool_file_args->lock = lock;
                                pool_file_args->data_num = 0;
                                pool_file_args->get_num_flag = true;
                                pool_file_args->run_flag = false;
                                pool_file_args->end_flag = false;
                                pool_file_args->datafd_to_file_args = new_args->datafd_to_file_args;
                                if (*new_args->FTP_data_stor_flag)
                                {
                                    pool_file_args->stor_flag = true;
                                    pool_file_args->retr_flag = false;
                                    pool_file_args->filename = *new_args->file_name;
                                    *new_args->FTP_data_stor_flag = false;
                                }
                                else if (*new_args->FTP_data_retr_flag)
                                {
                                    pool_file_args->stor_flag = false;
                                    pool_file_args->retr_flag = true;
                                    pool_file_args->filename = *new_args->file_name;
                                    *new_args->FTP_data_retr_flag = false;
                                }
                                else
                                {
                                    cout << "FTP_data_flag error" << endl;
                                    close(datafd);
                                    continue;
                                }
                                (*new_args->datafd_to_file_args)[datafd] = pool_file_args;

                                struct epoll_event ev;
                                ev.data.fd = datafd;
                                ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                                epoll_ctl(new_args->epfd, EPOLL_CTL_ADD, datafd, &ev);
                                send(datafd, new_args->client_num.c_str(), new_args->client_num.size() + 1, 0);
                                cout << "send-1" << endl;
                            }
                            else
                            {
                            }

                            pthread_cond_signal(new_args->recv_cond);

                            continue;
                        }
                        else if (recvjson["sort"] == MESSAGE)
                        {
                            if (recvjson["request"] == ASK_ADD_FRIEND)
                            {
                                cout << "\r\033[K" << flush;
                                cout << recvjson["message"] << endl;
                                rl_on_new_line();
                                if (*new_args->rl_display_flag)
                                    rl_redisplay();
                            }
                            else if (recvjson["request"] == ADD_BLACKLIST)
                            {
                                *new_args->pri_chat_flag = false;
                                cout << "\r\033[K" << flush;
                                cout << recvjson["reflact"] << endl;
                                rl_on_new_line();
                                if (*new_args->rl_display_flag)
                                    rl_redisplay();
                            }
                            else if (recvjson["request"] == GET_OFFLINE_MSG)
                            {
                                json elements = json::array();
                                elements = recvjson["elements"];
                                int count = elements.size();
                                if (count > 0)
                                {
                                    cout << "以下是新消息: " << endl;
                                    for (int i = 0; i < count; i++)
                                    {
                                        cout << elements[i] << endl;
                                    }
                                }
                            }
                            else if (recvjson["request"] == PEER_CHAT)
                            {
                                string sender = recvjson["sender"];
                                string receiver = recvjson["receiver"];
                                string message = recvjson["message"];
                                cout << "\r\033[K" << flush;
                                cout << "[" << sender << "->" << receiver << "]: ";
                                cout << message << endl;
                                rl_on_new_line();
                                if (*new_args->rl_display_flag)
                                    rl_redisplay();
                            }
                            else if (recvjson["request"] == NON_PEER_CHAT)
                            {
                                string message = recvjson["message"];
                                cout << "\r\033[K" << flush;
                                cout << message << endl;
                                rl_on_new_line();
                                if (*new_args->rl_display_flag)
                                    rl_redisplay();
                            }
                            else if (recvjson["request"] == START_FILE)
                            {
                                bool run_flag = recvjson["run_flag"];
                                int data_num = recvjson["data_num"];
                                file_args *file_pair = NULL;
                                file_pair = find_data_num_pair(data_num, new_args->datafd_to_file_args);
                                if (file_pair == nullptr)
                                {
                                    cout << "client can not find file_pair" << endl;
                                    pthread_cond_signal(file_pair->cond);
                                }
                                if (run_flag)
                                    file_pair->run_flag = true;
                                else
                                    cout << recvjson["reflact"] << endl;
                                pthread_cond_signal(file_pair->cond);
                            }
                            else if (recvjson["request"] == RETR_START)
                            {
                                bool retr_flag = recvjson["retr_flag"];
                                int data_num = recvjson["data_num"];
                                cout << recvjson["reflact"] << endl;
                                file_args *file_pair = NULL;
                                file_pair = find_data_num_pair(data_num, new_args->datafd_to_file_args);
                                if (!retr_flag)
                                    file_pair->retr_flag = false;
                                pthread_cond_signal(file_pair->cond);
                            }
                            else if (recvjson["request"] == STOR_START)
                            {
                                bool stor_flag = recvjson["stor_flag"];
                                int data_num = recvjson["data_num"];
                                cout << recvjson["reflact"] << endl;
                                file_args *file_pair = NULL;
                                file_pair = find_data_num_pair(data_num, new_args->datafd_to_file_args);
                                if (!stor_flag)
                                    file_pair->stor_flag = false;
                                pthread_cond_signal(file_pair->cond);
                            }
                            else if (recvjson["request"] == END_FILE)
                            {
                                bool end_flag = recvjson["end_flag"];
                                int data_num = recvjson["data_num"];
                                cout << recvjson["reflact"] << endl;
                                file_args *file_pair = NULL;
                                file_pair = find_data_num_pair(data_num, new_args->datafd_to_file_args);
                                if (end_flag)
                                    file_pair->end_flag = true;
                                pthread_cond_signal(file_pair->cond);
                            }
                            continue;
                        }
                        else if (recvjson["sort"] == ERROR)
                        {
                            cout << "\r\033[K" << flush;
                            cout << "发生错误 : " << recvjson["reflact"] << endl;
                            rl_on_new_line();
                            if (*new_args->rl_display_flag)
                                rl_redisplay();
                            *new_args->end_flag = true;
                            *new_args->end_start_flag = true;
                            *new_args->end_chat_flag = true;
                            pthread_cond_signal(new_args->recv_cond);
                            return nullptr;
                        }
                        else
                        {
                            cout << "\r\033[K" << flush;
                            cout << "接受信息类型错误: " << recvjson["sort"] << endl;
                            rl_on_new_line();
                            if (*new_args->rl_display_flag)
                                rl_redisplay();
                            *new_args->end_flag = true;
                            *new_args->end_start_flag = true;
                            *new_args->end_chat_flag = true;
                            pthread_cond_signal(new_args->recv_cond);
                            return nullptr;
                        }
                    }
                }
            }

            memset(recvbuf, 0, MAXBUF);
        }

        return nullptr;
    }
};