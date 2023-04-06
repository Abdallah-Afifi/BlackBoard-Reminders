#include <iostream>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <cstdlib>

// Replace <BLACKBOARD_URL>, <COURSE_ID>, <SESSION_ID>, and <COOKIE> with your own values
const std::string BLACKBOARD_URL = "https://blackboard.aucegypt.edu";
const std::string COURSE_ID = "_98160_1";
const std::string SESSION_ID = "7C024C7131FE373D489D8A3468234B9F";
const std::string COOKIE = "JSESSIONID=7C024C7131FE373D489D8A3468234B9F; BbRouter=7C024C7131FE373D489D8A3468234B9F";

// Fetch due dates from Blackboard
std::vector<std::string> fetch_due_dates() {
    std::vector<std::string> due_dates;
    std::string command = "curl -s -X GET --header \"Accept: application/json, text/plain, */*\" --header \"Accept-Language: en-US,en;q=0.5\" --header \"Content-Type: application/json;charset=utf-8\" --header \"Referer: " + BLACKBOARD_URL + "/ultra/course\" --header \"Cookie: " + COOKIE + "\" --header \"X-Blackboard-XSRF: 1\" \"" + BLACKBOARD_URL + "/learn/api/public/v2/courses/" + COURSE_ID + "/contents?fields=contents.v1%3Aadaptive-release,contentDetails%2CgradableItems&expand=contents.v1%3Aadaptive-release%2CgradableItems.attemptedSubmissions%2Ccontents.v1%3Achildren\" | jq -r \".results[] | select(has(\\\"gradableItems\\\")) | .gradableItems[] | select(has(\\\"dueDate\\\")) | .dueDate\"";
    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        std::cout << "Failed to fetch due dates from Blackboard." << std::endl;
    }
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
        std::string due_date(buffer);
        due_date.erase(due_date.find_last_not_of(" \t\r\n") + 1);
        due_dates.push_back(due_date);
    }
    _pclose(pipe);
    return due_dates;
}

// Create reminders for due dates
void create_reminders(const std::vector<std::string>& due_dates) {
    for (const auto& due_date : due_dates) {
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::tm tm_now = {};
        localtime_s(&tm_now, &now);
        std::tm tm_due_date = {};
        std::istringstream ss(due_date);
        ss >> std::get_time(&tm_due_date, "%Y-%m-%dT%H:%M:%S.%fZ");
        std::time_t due_date_time = std::mktime(&tm_due_date);
        std::chrono::hours hours_diff = std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::from_time_t(due_date_time) - std::chrono::system_clock::from_time_t(now));
        if (hours_diff.count() > 0 && hours_diff.count() < 168) {
            std::string command = "powershell -Command \"New-BurntToastNotification -Text 'You have an assignment due in " + std::to_string(hours_diff.count() / 24) + " days.'\"";
            system(command.c_str());
        }
    }
}

// Main function
int main() {
    std::vector<std::string> due_dates = fetch_due_dates();
    create_reminders(due_dates);
    return 0;
}

