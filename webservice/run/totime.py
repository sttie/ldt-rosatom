import json
import datetime


def transdate(srl_no):
    new_date = datetime.datetime(1899,12,29,0,0) + datetime.timedelta(srl_no - 1)
    return new_date

f = open('schedule_raw.json')
data = json.load(f)

for item in data["icebreakers"]:
    for path in item["path"]:
        path["end_time"] = str(transdate(path["end_time"]))
        path["start_time"] = str(transdate(path["start_time"]))

if "ships" in data.keys():
    for item in data["ships"]:
        for path in item["path"]:
            path["end_time"] = str(transdate(path["end_time"]))
            path["start_time"] = str(transdate(path["start_time"]))

with open('schedule.json', 'w') as f:
    json.dump(data, f, indent=2, ensure_ascii=False)