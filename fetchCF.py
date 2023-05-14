import requests
import json
import os
import time

import lzma
import pickle

files = os.listdir('./data')
print("Files", len(files))

for i in range(1, 1900):
    contestFileName = f"codeforces-{i}.xz"
    contestFilePath = f"./data/{contestFileName}"
    print("Checking", contestFileName, "??")
    if contestFileName not in files:
        ratingStatus = False
        try:
            ratingChanges = json.loads(requests.get(f'https://codeforces.com/api/contest.ratingChanges?contestId={i}').text)
            time.sleep(1.000)
            if ratingChanges['status'] == 'OK' and len(ratingChanges['result']) > 0:
                ratingStatus = True
        except:
            print("failed to check rating changes")
        res = requests.get(f'https://codeforces.com/api/contest.standings?contestId={i}&from=1&&showUnofficial=false')
        time.sleep(1.000)
        print(contestFilePath, len(res.text), "Rated" if ratingStatus else "Unrated")

        contestJson = json.loads(res.text)
        contestJson['rated'] = ratingStatus
        if('status' in contestJson and contestJson['status'] != 'FAILED'):
            data = json.dumps(contestJson)
            with lzma.open(contestFilePath, "wb") as f:
                pickle.dump(data, f)
            # with open(contestFilePath, 'w') as f:
            #     f.write(data)
        else:
            print(contestJson)