import sys
import json

def parse_edges(schedule):
    edges = {}
    for edge in schedule["edges"]:
        if not edge["start"] in edges.keys():
            edges[edge["start"]] = set()
        if not edge["end"] in edges.keys():
            edges[edge["end"]] = set()

        edges[edge["start"]].add(edge["end"])
        edges[edge["end"]].add(edge["start"])
    
    return edges

def edge_exists(start, end, edges):
    return start == end or (start in edges.keys() and end in edges[start])

def validate_icebreakers_path(icebreakers, edges):
    for icebreaker in icebreakers:
        _id, name, path = icebreaker["id"], icebreaker["name"], icebreaker["path"]
        
        start, end = path[0]["start"], path[0]["end"]
        # assert start == icebreaker's start from dataset

        for i in range(1, len(path)):
            assert edge_exists(start, end, edges), f"there's no edge between {start} and {end}"
            assert end == path[i]["start"], f"{i-1}th voyage's end doesn't equal to {i}th voyage's start for icebreaker {name}({_id})"
            start, end = path[i]["start"], path[i]["end"]

        # assert end == icebreaker's end from dataset

def build_path_for_ship(ship, icebreakers):
    _id, path = ship["id"], ship["path"].copy()

    for icebreaker in icebreakers:
        for voyage in icebreaker["path"]:
            if _id in voyage["caravan"]:
                path.append(voyage.copy())
                path[-1]["icebreaker"] = icebreaker["id"]

    return path

def validate_ships_path(ships, icebreakers, edges):
    for ship in ships:
        _id, name = ship["id"], ship["name"]

        # для начала формируем путь по каравану и путь в одиночку для каждого корабля
        path = build_path_for_ship(ship, icebreakers)
        path.sort(key=lambda voyage: (voyage["start_time"], voyage["end_time"]))

        for voyage in path:
            print(voyage)
        print("\n\n")

        start, end = path[0]["start"], path[0]["end"]
        # assert start == ship's start from dataset

        for i in range(1, len(path)):
            assert edge_exists(start, end, edges), f"there's no edge between {start} and {end}"
            assert end == path[i]["start"], f"{i-1}th voyage's end doesn't equal to {i}th voyage's start for ship {name}({_id})"
            start, end = path[i]["start"], path[i]["end"]

        # assert end == ship's end from dataset
        

def main():
    schedule_file = open(sys.argv[1], "r")
    schedule = json.load(schedule_file)
    edges = parse_edges(schedule)

    validate_icebreakers_path(schedule["icebreakers"], edges)
    validate_ships_path(schedule["ships"], schedule["icebreakers"], edges)

    schedule_file.close()

main()
