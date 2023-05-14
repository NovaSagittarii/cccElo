import math
import os

extra = os.listdir('./extra')
files = os.listdir('./fdata')
RESULTS_PATH = './fdata'

MIN_PARTICIPANTS = 10
CENTER = 1000 # updated based on RATEDBOUND

contests = []
users = {}

print("Initializing")
for fileName in files:
    # print(fileName)
    f = open(os.path.join(RESULTS_PATH, fileName), "r")

    """AC abc042
    1200 1469275200000
    AtCoder Beginner Contest 042
    4 100 200 300 400

    tuna_salad 1000 1416
    phirasit 1000 1743
    """
    lines = [line.strip() for line in f.read().splitlines()]
    if len(lines) <= 5: continue

    PLATFORM, CONTESTID = lines[0].split()
    RATEDBOUND, TIME = map(int, lines[1].split())
    CONTESTNAME = lines[2]
    CONTESTANTS = tuple(PLATFORM+"::"+line.split()[0] for line in lines[5:])
    contest = ((TIME, CONTESTNAME, RATEDBOUND, CONTESTANTS))
    contests.append(contest)
    f.close()

contests.sort(key=lambda x:x[0]) # sort by time
mostRecentContestContestants = set(contests[-1][1])
# print(contests)

for contestTime, contestName, RATEDBOUND, contestants in contests:
    CENTER = 1200
    if RATEDBOUND <= 2000: CENTER = 800
    elif RATEDBOUND <= 2800: CENTER = 1000

    print(contestTime, contestName, len(contestants), RATEDBOUND)
    APerf = []
    Perfs = []
    if len(contestants) < MIN_PARTICIPANTS: continue
    for contestant in contestants:
        if contestant not in users: APerf.append(CENTER)
        else:
            weightedSum, weightedTotal = 0, 0
            for i, performance in enumerate(reversed(users[contestant])):
                k = 0.9 ** (i+1)
                weightedSum += k * performance
                weightedTotal += k
            APerf.append(weightedSum / weightedTotal)
    for i, contestant in enumerate(contestants):
        # print(i)
        r = (i+1) - 0.5

        hi, lo, mid = 10000, 0, 0
        PRECISION = 1e-4
        while hi-lo > PRECISION:
            # print(lo,mid,hi)
            mid = lo + (hi-lo)/2.0
            lhs = sum( (1 + 6**((mid-aperf)/400))**-1 for aperf in APerf )
            if lhs > r: lo = mid
            elif lhs < r: hi = mid
            else: break
        # Perfs.append((contestant, mid))
        if contestant not in users: users[contestant] = [mid]
        else: users[contestant].append(mid)
    # print(Perfs)
    break
ratings = []

def geometricSeries(a1, r, n):
    if n >= 1e9: return a1/(1 - r)
    return a1*(1 - r**n)/(1 - r)
def F(x):
    return math.sqrt(geometricSeries(1, 0.81, x)) / geometricSeries(1, 0.9, x)
def f(x):
    return 1200 * (F(x)-F(1e9)) / (F(1)-F(1e9))
def g(x):
    return 2**(x / 800)
def gInv(x):
    return 800 * math.log2(x)

# (-inf, inf) -> (0, inf)
# @param {Number} [rating] unpositivized rating
# @returns {number} positivized rating
def positivizeRating(rating):
    if (rating >= 400.0): return rating
    return 400.0 * math.exp((rating - 400.0) / 400.0)
for user in users:
    contests = 0
    weightedSum = 0
    weightedTotal = 0
    for i, perf in enumerate(reversed(users[user])):
        if i == 0: perf = (perf - CENTER) * 1.5 + CENTER

        rperf = min(perf, RATEDBOUND + 400)
        k = (0.9) ** (i+1)
        weightedSum += g(rperf) * k
        weightedTotal += k
        contests += 1
    prevRating = -400 - f(1)
    if contests > 1:
        prevRating = gInv((weightedSum - g(users[user][-1]) )/(weightedTotal-1)) - f(contests-1)

    rating = gInv(weightedSum/weightedTotal) - f(contests)
    diff = int(positivizeRating(rating) - positivizeRating(prevRating))
    diffstr = ("+"+str(diff) if diff>0 else str(diff)) if user in mostRecentContestContestants else "---"
    # if diffstr == '0': diffstr = 'NC'
    ratingTuple = (user, positivizeRating(rating), diffstr, contests, [int(x) for x in users[user][::-1]][:5])
    ratings.append(ratingTuple)

print("MIN REQ", MIN_PARTICIPANTS, "TOTAL", len(ratings))

print("\033[4m\033[7m", "".rjust(3), "name".ljust(20), "elo".rjust(5), "Delta".rjust(5), "#".rjust(3), "history", " "*50, "\033[0m")

ratings.sort(key=lambda x:x[1], reverse=True)
for i, ratingTuple in enumerate(ratings[:20]):
    name, rating, diff, contestCount, x = ratingTuple
    rating = int(rating)
    inactive = diff == '---'
    # if(diff != "---"):
    print(("\033[90m" if inactive else "\033[0m") + str(i+1).rjust(3), name.ljust(20), str(rating).rjust(5), str(diff).rjust(5), str(contestCount).rjust(3), x)
print("\033[0m")