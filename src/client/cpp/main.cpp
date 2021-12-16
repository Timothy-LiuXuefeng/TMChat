#include <tmsocket/include/client_communicator.hpp>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <condition_variable>

int main(void)
{
    std::cout << "Please input server ip (127.0.0.1 by default): " << std::flush;
    std::string host;
    std::getline(std::cin, host);
    if (host == "") host = "127.0.0.1";
    std::cout << "Please input server port: " << std::flush;
    std::string port;
    std::getline(std::cin, port);
    tmsocket::client_communicator cs;


    cs.add_log([](const std::string& str) { std::cout << str << std::endl; });
    cs.on_connect
    (
        [&]()
        {
            std::cout << "Successfully connected to server!" << std::endl;
        }
    );

    cs.on_reveive([](const std::string& str) { std::cout << "Received: " << str << std::endl; });

    bool finished_connect = false;
    std::mutex finished_connect_mtx;
    std::condition_variable cond;

    cs.on_connect([&]
    {
        std::unique_lock<std::mutex> lock(finished_connect_mtx);
        finished_connect = true;
        cond.notify_all();
    });

    auto unexpected = std::make_shared<std::atomic_bool>(false);

    std::thread thr
    {
        [&, unexpected]
        {
            try
            {
                {
                    std::unique_lock<std::mutex> lock(finished_connect_mtx);
                    cond.wait(lock, [&] { return finished_connect; });
                }

                for (int i = 0; i < 10; ++i)
                {
                    if (*unexpected) return;
                    //                 tmsocket_postfixtmsocket_endmsgtmsocket_prefixtmsocket_prefixtmsocket_postfixtmsocket_postfixtmsocket_postfixtmsocket_endmsg
                    cs.send_to_server("tmsocket_postfixtmsocket_endmsgtmsocket_prefixtmsocket_prefixtmsocket_postfixtmsocket_postfixtmsocket_postfixtmsocket_endmsg");
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }

                cs.end_communication();
            }
            catch (const std::exception& e)
            {
                std::cout << e.what() << std::endl;
            }
        }
    };

    try
    {
        cs.connect(host, port);
    }
    catch(const tmsocket::disconnect_unexpectedly& e)
    {
        std::cout << e.what() << std::endl;
        cs.end_communication();
        *unexpected = true;
        thr.detach();
        return 1;
    }

    thr.join();
    return 0;
}
