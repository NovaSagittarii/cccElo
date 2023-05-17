#include "./parlaylib/include/parlay/parallel.h"
#include "calculator.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <vector>
#include <map>
#include <regex>
#include <set>
#include <utility>


#include <fstream>
#include <filesystem>

bool peekContestFile(const std::string& path, Contest& contest, const bool officialContest){
    std::fstream fs;
    fs.open(path, std::fstream::in);
    std::string type, id;
    int ratedBound;
    long long time = 0;
    bool success = false;
    if(fs >> type >> id >> ratedBound >> time){
        // std::cout << id << std::endl;
        contest.name = type + " " + id;
        contest.path = path;
        contest.processedPath = std::regex_replace(path, std::regex("/[^/]+/"), "/cache/");
        contest.time = time;
        contest.RatedBound = ratedBound;
        contest.official = officialContest;
        if(ratedBound <= 2000)      contest.Center = 800;
        else if(ratedBound <= 2800) contest.Center = 1000;
        else                        contest.Center = 1200;
        success = true;
    }
    fs.close();
    return success;
}

// Note: currently does not know if cache is outdated or wrong (would break if you add a contest that isnt at the end)
void processContest(Contest& contest, std::map<std::string, User>& users){
    // has this contest been processed before?
    bool cachedResult = false;
    {
        std::fstream f;
        f.open(contest.processedPath);
        if(f) cachedResult = true;
    }

    /// @brief array of AveragePerformance, used to determine performance
    std::vector<double> APerf;
    
    /// @brief <score, username>
    std::vector<std::pair<int, std::string>> standings;

    std::fstream fs;
    fs.open(contest.path, std::fstream::in);
    std::string type, id, contestName;
    int ratedBound = -1, problems = 0;
    long long time;
    
    fs >> type >> id >> ratedBound >> time;
    fs.ignore();
    std::getline(fs, contestName);
    fs >> problems;
    std::vector<int> problemWeights(problems);
    for(int i = 0; i < problems; i ++) fs >> problemWeights[i];

    // std::cout << "[" << time << "] " << contest.path << " // " << contestName << " ( ~ " << ratedBound << ")" << std::endl;

    int maxScore = 0;
    for(int x : problemWeights) maxScore += x;

    // ===== Get average performances of participants
    std::set<std::string> ratedUsers; // only will be used if contest is unofficial
    
    std::fstream fc; // file cached
    if(cachedResult){
        fc.open(contest.processedPath, std::fstream::in);
        fc.ignore(1000, '\n'); // type id
        fc.ignore(1000, '\n'); // ratedbound time
        fc.ignore(1000, '\n'); // title
        fc.ignore(1000, '\n'); // problems
        fc.ignore(1000, '\n'); // problemWeights[i]
        int n;
        fc >> n;
        for(int i = 0; i < n; i ++){
            double x;
            fc >> x;
            APerf.push_back(x);
        }
        for(int i = 0; i < n; i ++){
            int score;
            std::string name;
            fc >> score >> name;
            standings.push_back(std::make_pair(score, name));
        }
    }
    if(!cachedResult || !contest.official){ // always read for noncached or unofficial
        std::string username;
        int score, penalty;
        while(fs >> username >> score >> penalty){ // tuna_salad 1000 1416
            int w = 2e9 / maxScore;
            standings.push_back(std::make_pair(w*score - std::min(w*maxScore-1, penalty), username));
            ratedUsers.insert(username);
            if(users.count(username)){
                double aPerf = users[username].performance;
                // filter out people who are OUT_OF_COMPETITION
                if(aPerf < contest.RatedBound) APerf.push_back(aPerf);
                else{
                    standings.pop_back();
                    ratedUsers.erase(username);
                }
            }else APerf.push_back(contest.Center);
        }
    }
    fs.close();
    
    // ===== Calculate performance
    std::sort(APerf.begin(), APerf.end());
    std::sort(standings.begin(), standings.end(), [](const auto& lhs, const auto& rhs){
        return lhs.first > rhs.first;
    });
    std::vector<std::pair<double, std::vector<std::string>>> calculations;
    int j = 0; // left pointer
    for(int i = 0; i < (int)standings.size(); i ++){ // frontier
        if(standings[i].first == standings[j].first && i+1 < (int)standings.size()){
            // just advance pointer if you find same score
        }else{ // accumulate
            double position = 1 + ((double)i-1 + j)/2;
            calculations.push_back(std::make_pair(position, std::vector<std::string>()));
            for(int k = j; k < i; k ++) calculations.back().second.push_back(standings[k].second);
            j = i;
        }
    }
    
    if(!contest.official){ // filter out the official participants (don't need to recalculate for them)
        auto newCalculations = calculations;
        newCalculations.clear();
        for(auto [position, userList] : calculations){
            std::vector<std::string> newUserList;
            for(std::string user : userList){
                if(ratedUsers.count(user)) newUserList.push_back(user);
            }
            if(!newUserList.empty()) newCalculations.push_back(std::make_pair(position, newUserList));
        }
        calculations = newCalculations;
    }

    if(cachedResult && contest.official){
        int n;
        fc >> n;
        calculations.clear();
        while(n --){
            // -139.234140515327 582 Noob7
            double performance;
            int numPersons;
            fc >> performance >> numPersons;
            std::vector<std::string> persons(numPersons);
            for(int i = 0; i < numPersons; i ++) fc >> persons[i];
            calculations.push_back(std::make_pair(performance, persons));
        }
    }
    if(!cachedResult || !contest.official){
        // for(auto& [pos, users] : calculations){
        parlay::parallel_for(0, calculations.size(), [&](size_t i) {
            auto& [pos, users] = calculations[i];
            double hi = 10000, lo = -10000, mid = 0;
            const double PRECISION = 1e-4;
            const double rhs = pos - 0.5;
            while(hi-lo > PRECISION){
                mid = lo + (hi-lo)/2.0;
                double lhs = 0;
                for(auto aperf : APerf)
                    lhs += 1 / (1 + std::pow(6.0, (mid-aperf)/400.0));
                if(lhs > rhs) lo = mid;
                else if(lhs < rhs) hi = mid;
                else break;
            }
            mid = std::min(mid, contest.RatedBound + 400);
            pos = mid;
        });
    }

    contest.participants.clear();
    for(const auto [rating, ratedUsers] : calculations){
        // std::cout << rating << " " << ratedUsers[0] << std::endl;
        for(const std::string user : ratedUsers){
            // std::cout << rating << " " << user << std::endl;
            contest.participants.push_back(user);
            if(users.count(user) == 0){
                users[user] = User();
                users[user].contests = 0;
                users[user].name = user;
                users[user].performance = rating;
                users[user].performanceHistory = std::vector(1, rating);
            }else{
                const int contests = users[user].contests;
                const double p = users[user].performance;
                users[user].performance = (p * math::geometricSequence(0.9, 0.9, contests) + rating) * 0.9 / math::geometricSequence(0.9, 0.9, contests+1);
                users[user].contests ++;
                users[user].performanceHistory.push_back(rating);
            }
        }
    }

    // === save the results into the processed file path
    if(contest.official && !cachedResult){
        fs.open(contest.processedPath, std::fstream::out | std::fstream::app);
        fs << type << " " << id << "\n" << ratedBound << " " << time << "\n" << contestName << "\n" << problems << "\n";
        for(int i = 0; i < problems; i ++) fs << problemWeights[i] << " ";
        fs << "\n\n" << standings.size() << "\n";
        for(auto aperf : APerf) fs << aperf << " ";
        fs << "\n";
        for(auto [score, name] : standings) fs << score << " " << name << "\n";
        fs << "\n" << calculations.size() << "\n";
        for(auto calculation : calculations){
            fs << std::setprecision(std::numeric_limits<double>::digits10) << calculation.first << " " << calculation.second.size();
            for(auto x : calculation.second) fs << " " << x;
            fs << "\n";
        }
    }
}

double calculateRating(const User& user){
    int contests = (int)user.performanceHistory.size();
    double k = 0.9;
    double weightedSum = 0;
    double weightedTotal = math::geometricSequence(0.9, 0.9, contests);
    for(int i = (int)user.performanceHistory.size()-1; i >= 0; i --){
        double perf = user.performanceHistory[i];
        weightedSum += k * math::g(perf);
        k *= 0.9;
    }
    double rating = math::gInv(weightedSum/weightedTotal)- math::f(contests);
    return math::positivizeRating(rating);
}

int main(){
    // std::ios_base::sync_with_stdio(false);
    // std::cin.tie(0);
    
    std::vector<Contest> contests;
    std::map<std::string, User> users;

    const std::string path = "./fdata";
    for (const auto & entry : std::filesystem::directory_iterator(path)){
        Contest contest;
        if(peekContestFile(entry.path(), contest, true)){
            contests.push_back(contest);
        }
    }
    for (const auto & entry : std::filesystem::directory_iterator("./extra")){
        Contest contest;
        if(peekContestFile(entry.path(), contest, false)) contests.push_back(contest);
    }
    std::sort(contests.begin(), contests.end(), [](const auto& lhs, const auto& rhs){
        return lhs.time < rhs.time;
    });
    
    std::set<std::string> unofficialUsers;
    // int ctn = 0;
    for(auto contest : contests){
        // if(++ctn > 2) break;
        // if(++ctn >= 40) break;
        // if(++ctn >= 200) break;
        processContest(contest, users);
        if(!contest.official){
            std::cout << contest.name << std::endl;
            for(auto user : contest.participants) unofficialUsers.insert(user);
        }
        // break;
    }
    // for(auto x : unofficialUsers) std::cout << x << " ";
    unofficialUsers.insert("Akatsuki_");

    std::vector<std::pair<int, std::string>> ratings;
    for(auto [username, user] : users){
        ratings.push_back(std::make_pair(calculateRating(user), username));
    }
    std::sort(ratings.begin(), ratings.end());
    std::reverse(ratings.begin(), ratings.end());

    int usersPrinted = 0;
    int i = 0;
    for(auto [rating, username] : ratings){
        i ++;
        if(!unofficialUsers.count(username)) continue;
        int percentileScore = (1.0 - (double)i / ratings.size()) * 10000;
        std::cout << std::left << std::setw(20) << username << std::setw(5) << rating << std::setw(6) << percentileScore << " === ";
        for(int i = 0; i < 4; i ++){
            int k = users[username].performanceHistory.size();
            if(i >= k) break;
            std::cout << " " << (int)users.at(username).performanceHistory[k-1-i];
        }

        std::cout << "\n";
        if(++usersPrinted >= 20) break;
    }

    return 0;
}