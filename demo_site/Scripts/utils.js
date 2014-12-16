function PrettyPrintDuration(dur) {
    var durSpec = [{ unit: "s", threshold: 1, div: 1 },
                   { unit: "m", threshold: 60, div: 60 },
                   { unit: "h", threshold: 60, div: 60 },
                   { unit: "d", threshold: 24, div: 24 },
                   { unit: "w", threshold: 7, div: 7 },
                   { unit: "M", threshold: 6, div: 4.345 },
                   { unit: "y", threshold: 12, div: 12}];
    var unit = "s";
    var val = dur;
    for (var i in durSpec) {
        if (val < durSpec[i]["threshold"]) {
            break;
        }
        val = val / durSpec[i]["div"];
        unit = durSpec[i]["unit"];
    }

    val = (Math.round(val * 100) / 100) + '';
    var m = val.match(/\.\d+/);
    if (m == null) {
        val += ".00";
    } else {
        if (m[0].length < 3) {
            val += "0";
        }
    }

    return val + " " + unit;
}
