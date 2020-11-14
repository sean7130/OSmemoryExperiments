# traces = ["tr-simpleloop.ref", "tr-matmul.ref",
#           "tr-blocked.ref", "tr-fourth.ref"]
traces = ["tr-blocked.ref"]
algs = ["rand", "fifo", "lru", "clock", "opt"]
memsizes = ["50", "100", "150", "200"]
num_cols = 6

for trace in traces:
    all_alg_metric = []
    for alg in algs:
        alg_metric = [None] * (len(memsizes) * num_cols)
        alg_metrics = []

        for mem in memsizes:
            filename = trace + "-" + alg + "-" + mem + ".txt"

            with open(filename) as f:
                content = f.readlines()

            content = [line.strip().split(":") for line in content]

            metrics_for_mem = [content[5][1].strip(), content[0][1].strip(),
                               content[1][1].strip(),
                               str(int(content[2][1].strip()) +
                                   int(content[3][1].strip())),
                               content[2][1].strip(), content[3][1].strip()]

            alg_metrics.append(metrics_for_mem)

        i = 0
        for am in alg_metrics:
            alg_metric[i::4] = alg_metrics[i]
            i = i + 1

        alg_metric = [alg.upper()] + alg_metric
        all_alg_metric.append(alg_metric)


    with open(trace + '-table.txt', 'a') as f:
        for am in all_alg_metric:
            f.write(" & ".join(am) + "\n")
