#include <iostream>
#include <curl/curl.h>
#include <thread>
#include <chrono>
#include <windows.h>
#include <string>
#include <regex>

// Функция для обработки ответа и сохранения его в строку
size_t write_callback(void* ptr, size_t size, size_t nmemb, void* userdata) {
    std::string* response = static_cast<std::string*>(userdata);
    response->append(static_cast<char*>(ptr), size * nmemb);
    return size * nmemb;
}

// Функция для получения заголовка страницы (тега <title>)
std::string getTitle(const std::string& url) {
    CURL* curl = curl_easy_init();
    std::string response;
    std::string title = "Неизвестно";

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            std::regex title_regex("<title>(.*?)</title>", std::regex::icase);
            std::smatch match;
            if (std::regex_search(response, match, title_regex)) {
                title = match[1].str();
            }
        }

        curl_easy_cleanup(curl);
    }
    
    return title;
}

// Функция для отправки запроса и вывода HTTP-кода
long sendRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    long http_code = 0;
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L); // Запрос без тела (получаем только заголовки)
        
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK) {
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
            std::cout << "HTTP-код ответа: " << http_code << std::endl;
        } else {
            std::cerr << "Ошибка cURL: " << curl_easy_strerror(res) << std::endl;
        }
        
        curl_easy_cleanup(curl);
    }
    
    return http_code;
}

int main() {
    SetConsoleOutputCP(65001); // Устанавливаем UTF-8 в консоли Windows

    std::cout << "=== ВНИМАНИЕ ===" << std::endl;
    std::cout << "Эта программа предназначена только для обучения и тестирования." << std::endl;
    std::cout << "Разработчик не несет ответственности за нанесенный ущерб." << std::endl;
    std::cout << "=================" << std::endl;
    
    while (true) {
        std::string url;
        int count;
        
        std::cout << "Введите URL|IP-адрес (или 'exit' для выхода): ";
        std::getline(std::cin, url);
        if (url == "exit") break;

        // Получаем заголовок страницы
        std::string title = getTitle(url);
        std::cout << "Вебсайт: " << title << std::endl;

        std::cout << "Введите количество запросов (-1 для бесконечного режима): ";
        std::cin >> count;
        std::cin.ignore(); // Убираем лишний перевод строки после std::cin

        int successCount = 0, failCount = 0;

        if (count == -1) {
            std::cout << "Запуск в бесконечном режиме..." << std::endl;
            while (true) {
                long code = sendRequest(url);
                if (code >= 200 && code < 300) successCount++;
                else failCount++;

                std::cout << "Успешных запросов: " << successCount << " | Ошибок: " << failCount << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        } else {
            for (int i = 0; i < count; ++i) {
                long code = sendRequest(url);
                if (code >= 200 && code < 300) successCount++;
                else failCount++;

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            std::cout << "Всего запросов: " << count << std::endl;
            std::cout << "Успешных запросов (2xx): " << successCount << std::endl;
            std::cout << "Ошибок (4xx, 5xx): " << failCount << std::endl;
            std::cout << "============================\n";
        }
    }

    std::cout << "Программа завершена." << std::endl;
    return 0;
}
