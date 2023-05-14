import requests
import json
import os
import time

import lzma
import pickle

cookies = { # need to fill in
    '_ga' :'',
    '_gid':'',
    'REVEL_SESSION':"",
    'Path': '/',
    'Expires': '',
    'Max-Age': '',
}

files = os.listdir('./data')
contests = []
with open("./ndata/atcoder-contests.json") as f: contests = f.read()
contests = json.loads(contests)

for contest in contests:
    # print(contest)
    id = contest['id']
    start = contest['start_epoch_second']
    title = contest['title']
    rating = contest['rate_change']
    if rating == '-': continue
    # print(id, rating)
    print(id, title)

    contestFileName = f"atcoder-{id}.xz"
    contestFilePath = f"./data/{contestFileName}"
    if contestFileName not in files:
        res = requests.get(f'https://atcoder.jp/contests/{id}/standings/json', cookies=cookies)
        # print (res.text[:50].strip())
        # break
        if res.text.strip().startswith("<!DOCTYPE html>"): continue
        print(contestFilePath, len(res.text))
        resJson = json.loads(res.text)
        if 'TaskInfo' in resJson:
            with lzma.open(contestFilePath, "wb") as f:
                pickle.dump(res.text, f)
            # with open(contestFilePath, 'w') as f:
            #     f.write(res.text)
            time.sleep(1.000)
        else:
            print(resJson)
   