#ifndef __CCCELO_CALCULATOR_H__
#define __CCCELO_CALCULATOR_H__

#include <string>
#include <vector>
#include <map>

/// @brief container for representing the results of a contest, can be incomplete (before read) or completed (after read)
struct Contest {
    /// @brief name of the contest
    std::string name;
    /// @brief full name of the contest
    std::string fullName;
    /// @brief path to the file of the contest file (to reopen when needed)
    std::string path;
    /// @brief path to the file of the contest file, processed results
    std::string processedPath;
    /// @brief time in seconds since Unix epoch
    long long time;
    /// @brief RATEDBOUND of the contest
    double RatedBound;
    /// @brief CENTER of the contest
    double Center;
    /// @brief whether this is a real contest (official) or an extra one (unofficial)
    bool official;
    /// @brief who participated
    std::vector<std::string> participants;
};

/// @brief container representing a single user
struct User {
    /// @brief name of the user (identifier)
    std::string name;
    /// @brief Average Performance
    double performance;
    /// @brief history of performance (back of vector is most recent)
    std::vector<double> performanceHistory;
    /// @brief how many contests this contestant has participated in
    int contests;
};

/// @brief read just the headers of a file to determine its start date (to be sorted)
/// @param path path of the file to peek
/// @param contest where result data will be written into
/// @param officialContest whether the contest is "official" or not
/// @return if reading was successful
bool peekContestFile(const std::string&, Contest&, const bool);

/// @brief process the contest results and update users, creates a file for the results afterwards
/// @param contest contest object to process
/// @param users list of users operations are applied to
void processContest(Contest&, std::map<std::string, User>&);

/// @brief calculate the rating of a user from their performance
/// @param user user to calculate for
/// @return user's rating
double calculateRating(const User&);

/// @brief math functions for rating calculations
namespace math {
    /// @brief sum of geometric series
    /// @param a1 first number in the series
    /// @param r geometric ratio
    /// @param n number of elements
    /// @return value of the sequence
    double geometricSequence(double, double, double);

    /// @brief auxillary function used to calculate @fn f the low contest penalty function
    /// @param x 
    /// @return value of F(x)
    double F(double);

    /// @brief low contest penalty function
    /// @param x how many contests someone has taken part in
    /// @return rating penalty
    double f(double);

    /// @brief performance weighting function
    /// @brief "This function is used to assign more weights to better performances.
    /// @brief Thus, there is a big difference between very good performance and moderately
    /// @brief good performance, while the difference between big failure and moderate failure is not so big"
    /// @param x
    /// @return 2**(x/800)
    double g(double);

    /// @brief inverse of the performance weighting function g(x)
    /// @param x
    /// @return y : 2**(y/800) = x
    double gInv(double);

    /// @brief (-inf, inf) -> (0, inf)
    /// @param rating unpositivized rating
    /// @return positivized rating
    double positivizeRating(double);
};
#endif