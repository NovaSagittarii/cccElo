#include "./Parallel-Quicksort/parallel.h"
#include "./Parallel-Quicksort/get_time.h"

#include "calculator.h"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <vector>
#include <map>
#include <utility>

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

bool peekContestFile(const std::string& path, Contest& contest){
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
        contest.time = time;
        contest.RatedBound = ratedBound;
        if(ratedBound <= 2000)      contest.Center = 800;
        else if(ratedBound <= 2800) contest.Center = 1000;
        else                        contest.Center = 1200;
        success = true;
    }
    fs.close();
    return success;
}

void processContest(Contest& contest, std::map<std::string, User> users){
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

    std::cout << time << ") " << contest.path << " // " << " " << contestName << std::endl;

    int maxScore = 0;
    for(int x : problemWeights) maxScore += x;

    // ===== Get average performances of participants
    std::string username;
    int score, penalty;
    while(fs >> username >> score >> penalty){ // tuna_salad 1000 1416
        int w = 2e9 / maxScore;
        standings.push_back(std::make_pair(w*score - std::min(w*maxScore-1, penalty), username));
        if(users.count(username)){
            double aPerf = users[username].performance;
            // filter out people who are OUT_OF_COMPETITION
            if(aPerf < contest.RatedBound) APerf.push_back(aPerf);
            else standings.pop_back();
        }else APerf.push_back(contest.Center);
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
    for(auto [rating, users] : calculations){
        for(auto user : users){
            std::cout << rating << " " << user << std::endl;
        }
    }
}

int main(){
    // std::ios_base::sync_with_stdio(false);
    // std::cin.tie(0);
    
    std::vector<Contest> contests;
    std::map<std::string, User> users;

    const std::string path = "./fdata";
    for (const auto & entry : fs::directory_iterator(path)){
        Contest contest;
        if(peekContestFile(entry.path(), contest)){
            contests.push_back(contest);
        }
    }
    std::sort(contests.begin(), contests.end(), [](const auto& lhs, const auto& rhs){
        return lhs.time < rhs.time;
    });
    int ctn = 0;
    for(auto contest : contests){
        if(++ctn >= 200) break;
        processContest(contest, users);
        // break;
    }

    return 0;
}