import os
import subprocess
import itertools
import multiprocessing
import pandas as pd
import uuid
import seaborn as sns
import matplotlib.pyplot as plt
import argparse
import numpy as np
parser=argparse.ArgumentParser()
parser.add_argument("--vol-start",type=float)
parser.add_argument("--vol-stop",type=float)
parser.add_argument("--vol-step",type=float)
parser.add_argument("--spread-start",type=float)
parser.add_argument("--spread-stop",type=float)
parser.add_argument("--spread-step",type=float)
parser.add_argument("--seed",type=int,default=42)
args=parser.parse_args()
SIM_BIN = "../sim"
RESULTS_DIR="results"
os.makedirs(RESULTS_DIR,exist_ok=True)

def parse_metrics_csv(path):
    with open(path, "r") as f:
        lines = f.read().splitlines()

    bots = []
    metrics = {}
    for line in lines:
        if line == "Metric,Value":
            if metrics:
                bots.append(metrics)
                metrics = {}
        elif "," in line:
            k, v = line.split(",", 1)
            metrics[k.strip()] = v.strip()

    if metrics:
        bots.append(metrics)

    return pd.DataFrame(bots)

param_grid = {
    "SPREAD_THRESHOLD": np.arange(args.spread_start,args.spread_stop+args.spread_step,args.spread_step),
    "TRAIL_STOP_PCT": [0.002, 0.003, 0.005],
    "STOP_LOSS_PCT": [0.005, 0.01],
    "VOL": np.arange(args.vol_start,args.vol_stop+args.vol_step,args.vol_step),
}

def run_sim(params):
    uid = uuid.uuid4().hex[:8]
    config_path = f"{RESULTS_DIR}/config_{uid}.txt"
    result_path =f"{RESULTS_DIR}/metrics_{uid}.csv"
    config_str = "\n".join([
        f"SPREAD_THRESHOLD={params[0]}",
        f"TRAIL_STOP_PCT={params[1]}",
        f"STOP_LOSS_PCT={params[2]}",
        f"DEFAULT_VOLATILITY={params[3]}",
        "INITIAL_PRICE=100",
        "TIME_PER_TICK_MS=10",
        "MEAN_REVERSION_WINDOW=100",
        "SHORT_WINDOW=10",
        "LONG_WINDOW=50",
        "MINHOLDTIME_TICK=6",
        "PROXIMITY_SIGMA=0.3",
        "DEFAULT_DRIFT=0.05"
    ])
    with open(config_path, "w") as f:
        f.write(config_str)

    try:
        subprocess.run([SIM_BIN, "-t", "10000","--config",config_path,"--output",result_path], stdout=subprocess.DEVNULL,stderr=subprocess.PIPE,check=True, timeout=60)
        df=parse_metrics_csv(result_path)
        for col, val in zip(["SPREAD_THRESHOLD", "TRAIL_STOP_PCT", "STOP_LOSS_PCT", "VOL"], params):
            df[col] = val
        return df
    except Exception as e:
        print(f"Error with params {params}: {e}")
        return None
    finally:
        if os.path.exists(config_path):
            os.remove(config_path)

if __name__ == "__main__":
    all_combinations = list(itertools.product(
        param_grid["SPREAD_THRESHOLD"],
        param_grid["TRAIL_STOP_PCT"],
        param_grid["STOP_LOSS_PCT"],
        param_grid["VOL"],
    ))

    with multiprocessing.Pool(processes=os.cpu_count()) as pool:
        dfs=pool.map(run_sim, all_combinations)

    dfs = [df for df in dfs if df is not None]
    final_df = pd.concat(dfs, ignore_index=True)
    final_df.to_csv("sweep_results.csv", index=False)

    bots = final_df["Bot"].unique()
    for bot in bots:
        df_bot = final_df[final_df["Bot"] == bot].copy()
        df_bot.sort_values("Sharpe", ascending=False, inplace=True)
        df_bot.to_csv(f"sweep_{bot}.csv", index=False)
        print(f"\nTop résultats pour {bot}:\n", df_bot.head(3))
    print(final_df.head())
