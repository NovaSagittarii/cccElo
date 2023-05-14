import requests
import json
import os

import lzma
import pickle

files = os.listdir('./data')
formattedFiles = os.listdir('./fdata')

def readJsonFile(path):
    with open(path) as f:
        content = f.read()
    return json.loads(content)
def decodeXzFile(path):
    with lzma.open(path, "rb") as f:
        data = pickle.load(f)
    return json.loads(data)

# Codeforces
def formatCodeforces(skipFinished=True):
    for id in range(1, 1900):
        contestFileName = f"codeforces-{id}.xz"
        contestFilePath = f"./data/{contestFileName}"
        formattedFileName = f"codeforces-{id}.txt"
        formattedFilePath = f"./fdata/{formattedFileName}"
        if contestFileName not in files:
            print(f"missing contest -- {contestFilePath}")
            continue
        if formattedFileName in formattedFiles:
            if skipFinished: continue # TODO: implement sys.args -f
            # clear it (if forced)
            with open(formattedFilePath, "w") as f:
                f.truncate(0)
        ffile = open(formattedFilePath, "a")
        # result = readJsonFile(contestFilePath)
        result = decodeXzFile(contestFilePath)
        if 'status' not in result or result['status'] != 'OK': continue
        if 'result' not in result: continue
        if 'rated' not in result or not result['rated']: continue

        # get contest metadata
        contest = result['result']['contest']
        name = contest['name']
        startTime = contest['startTimeSeconds']*int(1e3)
        
        # figure out rating range
        RatedBounds = (9999, 2800, 1600, 1200)
        divs = [False for i in RatedBounds]
        for i in range(4):
            if f'Div. {i+1}' in name: divs[i] = True
        if not max(divs): divs[0] = True # make it Rated D1 (assume its like "global")
        for i in range(1, 4): divs[i] = max(divs[i], divs[i-1]) # extend ratedness
        ratedBound = max(divs[i]*rb for i, rb in enumerate(RatedBounds))

        print(id, name)
        ffile.write(f"CF {id}\n{ratedBound} {startTime}\n{name}\n")
        
        problemWeights = tuple(problem['points'] if 'points' in problem else (i+1)*100 for i, problem in enumerate(result['result']['problems']))
        ffile.write(f"{len(problemWeights)} {' '.join(str(int(pw)) for pw in problemWeights)}\n\n")
        
        # calculate standings
        participants = result['result']['rows']
        for participant in participants:
            usernames = tuple(member['handle'] for member in participant['party']['members'])
            score = 0
            penalty = 0
            # calculate scoring
            for i, problemResult in enumerate(participant['problemResults']):
                if problemResult['points'] > 0:
                    score += problemWeights[i]
                    penalty += problemResult['bestSubmissionTimeSeconds']
                    penalty += 5*60 * problemResult['rejectedAttemptCount']
            for username in usernames:
                ffile.write(f'{username} {int(score)} {int(penalty)}\n')
        ffile.close()
        # break


# Atcoder
def formatAtcoder(skipFinished=True):
    atcoderContests = readJsonFile("./ndata/atcoder-contests.json")
    for contest in atcoderContests:
        # get contest metadata
        id = contest['id']
        startTime = int(contest['start_epoch_second']*1e3)
        name = contest['title']
        rating = contest['rate_change']
        if rating == '-': continue
        
        # open files
        contestFileName = f"atcoder-{id}.xz"
        contestFilePath = f"./data/{contestFileName}"
        formattedFileName = f"atcoder-{id}.txt"
        formattedFilePath = f"./fdata/{formattedFileName}"
        if contestFileName not in files:
            print(f"missing contest -- {contestFilePath}")
            continue
        if formattedFileName in formattedFiles:
            if skipFinished: continue # TODO: implement sys.args -f
            # clear it (if forced)
            with open(formattedFilePath, "w") as f:
                f.truncate(0)
        ffile = open(formattedFilePath, "a")
        # result = readJsonFile(contestFilePath)
        result = decodeXzFile(contestFilePath)

        # figure out ratedBound
        ratedBound = 9999
        if '~' in rating: ratedBound = int(rating.split('~')[-1].strip() or 9998)+1

        print(id, name, rating)
        ffile.write(f"AC {id}\n{ratedBound} {startTime}\n{name}\n")

        # calculating the weights
        participants = result['StandingsData']
        problemIds = {}
        for i, task in enumerate(result['TaskInfo']): problemIds[task['TaskScreenName']] = i
        problemWeights = [0 for i in range(len(result['TaskInfo']))]
        pw = len(problemWeights)
        for participant in participants:
            for k, v in participant['TaskResults'].items():
                score = v['Score'] // 100
                if score > 0:
                    problemWeights[problemIds[k]] = score
        for i, problemWeight in enumerate(problemWeights):
            if problemWeight == 0:
                problemWeights[i] += 100 if i == 0 else problemWeights[i-1]
        fstr = f"{len(problemWeights)} {' '.join(str(int(pw)) for pw in problemWeights)}\n\n"
        ffile.write(fstr)
        
        # calculate standings
        for participant in participants:
            # participant['UserName'] or 
            username = participant['UserScreenName'] or 'deleted_account_41249' # why is there UserScreenName (its the actual username) (also there are blank names :moyai:)
            score = 0
            penalty = 0
            # calculate scoring
            for k, v in participant['TaskResults'].items():
                if v['Score'] > 0:
                    score += v['Score'] // 100
                    penalty += int(v['Elapsed'] // 1e9)
                    penalty += 5*60 * v['Penalty']
            ffile.write(f'{username} {int(score)} {int(penalty)}\n')
        ffile.close()

if __name__ == '__main__':
    formatCodeforces()
    formatAtcoder()