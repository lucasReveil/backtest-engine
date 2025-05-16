import dash
from dash import dcc, html, dash_table
from dash.dependencies import Input, Output
import pandas as pd
import plotly.graph_objects as go
import plotly.express as px
import os
import glob
import subprocess
from dash import State
from dash import callback_context
# Récupère le chemin absolu vers le dossier du script
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
data_dir = os.path.join(BASE_DIR, "../data")
window=100
long_window=50
short_window=10
def load_market_data():
    df=pd.read_csv(data_dir+"/market.csv")
    df['rolling_mean'] = df['price'].rolling(window=window).mean()
    df['long_sma'] = df['price'].rolling(window=long_window).mean()
    df['short_sma'] = df['price'].rolling(window=short_window).mean()
    df['rolling_std'] = df['price'].rolling(window=window).std()
    df['upper_band'] = df['rolling_mean'] + 2 * df['rolling_std']
    df['lower_band'] = df['rolling_mean'] - 2 * df['rolling_std']
    df['SL_long'] = df['rolling_mean'] - 3 * df['rolling_std']
    df['SL_short'] = df['rolling_mean'] + 3 * df['rolling_std']
    return df

def load_trades():
    trades = {}
    for filepath in glob.glob(os.path.join(data_dir, "*_trades.csv")):
        bot_name = os.path.basename(filepath).replace("_trades.csv", "")
        trades[bot_name] = pd.read_csv(filepath)
    return trades

def fill_graph_pnl():
    fig = go.Figure()
    for bot_name, df in dfTrades.items():
        if df.empty:
            continue
        fig.add_trace(go.Scatter(
        x=df['trade_id'],
        y=df['pnl'],
        mode='lines+markers',
        name=f"{bot_name} PnL"
        ))
    fig.update_layout(
        title="Cumulative PnL by Strategy",
        xaxis_title="Trade #",
        yaxis_title="PnL",
        height=400,
        legend_title="Bots",
        margin=dict(l=40,r=40,t=60,b=40),
        template="plotly_white"
    )
    return fig

def make_base_fig(df):
    """Construit la figure de base (prix)"""
    fig = go.Figure()
    fig.add_trace(go.Scatter(
        x=df['time'], y=df['price'],
        mode='lines', name='Price', line=dict(color='blue')
    ))
    return fig

def fig_add_bollinger(fig):
    global dfMarket
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['rolling_mean'],
        mode='lines', name='Rolling Mean',
        line=dict(dash='dash', color='orange')
    ))
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['SL_short'],
        mode='lines', name='Lower Band',
        line=dict(dash='dot', color='red')
    ))
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['SL_long'],
        mode='lines', name='Lower Band',
        line=dict(dash='dot', color='red')
    ))
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['upper_band'],
        mode='lines', name='Upper Band',
        line=dict(dash='dot', color='gray')
    ))
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['lower_band'],
        mode='lines', name='Lower Band',
        line=dict(dash='dot', color='gray'),
        fill='tonexty', fillcolor='rgba(128,128,128,0.1)'
    ))
    return fig

def fig_add_crossing_SMAs(fig):
    global dfMarket
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['long_sma'],
        mode='lines', name=f"Long_SMA({long_window})",
        line=dict(dash='dash', color='red')
    ))
    fig.add_trace(go.Scatter(
        x=dfMarket['time'], y=dfMarket['short_sma'],
        mode='lines', name=f"Short_SMA({short_window})",
        line=dict(dash='dash', color='green')
    ))
    return fig
dfMarket = load_market_data()
dfTrades = load_trades()
fig_pnl = fill_graph_pnl()
fig_base= make_base_fig(dfMarket)
external_stylesheets = ['https://codepen.io/chriddyp/pen/bWLwgP.css']
app = dash.Dash(__name__,suppress_callback_exceptions=True, external_stylesheets=external_stylesheets)
app.layout=html.Div([
    html.H1("Backtest Dashboard",style={"textAlign":"center"}),
    dcc.Tabs(id="tabs",value="simulation",children=[
        dcc.Tab(label="Live Simulation",value="simulation"),
        dcc.Tab(label="Parameter Sweep",value="sweep")
    ]),
    html.Div(id="tab-content")
])

simulation_tab_layout = html.Div([
    html.Div([
        html.H3("Simulation Parameters"),

        html.Label("Initial Price:"),
        dcc.Input(id='price-input', type='number', value=100.0),

        html.Label("Trail stop %"),
        dcc.Input(id='trailstop-input', type='number', value=0.3),

        html.Label("Number of ticks:"),
        dcc.Input(id='tick-input', type='number', value=10000),

        html.Label("Time per tick (ms):"),
        dcc.Input(id='ticktime-input', type='number', value=10),

        html.Label("Drift:"),
        dcc.Input(id='drift-input', type='number', value=0.05, step=0.01),

        html.Label("Volatility:"),
        dcc.Input(id='vol-input', type='number', value=0.3, step=0.01),

        html.Label("Seed:"),
        dcc.Checklist(
            id="fix-seed-toggle",
            options=[{"label": "Fix seed", "value": "fixed"}],
            value=[],
            labelStyle={"display": "inline-block"}
        ),
        dcc.Input(id="seed-input", type="number", value=42),

        html.Label("Mean Window"),
        dcc.Input(id='window-input', type='number', value=100),

        html.Label("Long Window"),
        dcc.Input(id='long-window-input', type='number', value=50),

        html.Label("Short Window"),
        dcc.Input(id='short-window-input', type='number', value=10),

        html.Label("Spread Threshold (BPS)"),
        dcc.Input(id='spread-threshold-input', type='number', value=0.4),

        html.Label("Min holdtime (ticks)"),
        dcc.Input(id='minhold-tick-input', type='number', value=5),

        html.Label("Proximity sigma"),
        dcc.Input(id='proximity-sigma-input', type='number', value=0.2),

        html.Label("Stop loss (%)"),
        dcc.Input(id='stop-loss-input', type='number', value=1.0),

        html.Br(),
        html.Button("Run Simulation", id='run-btn', n_clicks=0)

    ], style={"flex": "30%", "padding": "20px"}),

    html.Div([
        html.Label("Select bots to show trades:"),
        dcc.Checklist(
            id='bot-selector',
            options=[{'label':name,'value':name} for name in dfTrades.keys()],
            value=[],
            labelStyle={"display": "inline-block"}
        ),
        dcc.Graph(id="market-price", figure=fig_base),
        html.Hr(),
        dcc.Graph(id="pnl-graph", figure=fig_pnl)
    ], style={"flex": "70%", "padding": "20px"})

], style={"display": "flex"})


def sweep_tab_layout():
    return html.Div([
        html.Div([
            html.H4("Sweep Parameter"),
            html.Label("Vol range(start,stop,step)"),
            dcc.Input(id="vol-start",type="number",value=0.1),
            dcc.Input(id="vol-stop",type="number",value=0.3),
            dcc.Input(id="vol-step",type="number",value=0.1),

            html.Label("Spread threshold range(start,stop,step)"),
            dcc.Input(id="spread-start",type="number",value=0.2),
            dcc.Input(id="spread-stop",type="number",value=0.4),
            dcc.Input(id="spread-step",type="number",value=0.1),

            html.Label("Seed (optionnal)"),
            dcc.Input(id="sweep-seed",type="number",value=42),

            html.Button("Launch Sweep",id="launch-sweep",n_clicks=0),
            html.Div(id="sweep-status")
            ],id="sweep-form",style={"marginBottom":"30px"}),
        html.Div(id="sweep-results")
    ])
@app.callback(
    Output("sweep-heatmap", "figure"),
    Input("sweep-bot-select", "value")
)
def update_heatmap(bot_name):
    df = pd.read_csv("sweep_results.csv")
    df = df[df["Bot"] == bot_name]
    df = df.dropna()

    # Pivot heatmap (example: SPREAD_THRESHOLD vs VOL → Sharpe)
    pivot = df.pivot_table(
        index="SPREAD_THRESHOLD",
        columns="VOL",
        values="Sharpe",
        aggfunc="mean"
    )

    fig = px.imshow(pivot, text_auto=".2f", color_continuous_scale="RdBu_r")
    fig.update_layout(title=f"Heatmap Sharpe - {bot_name}")
    return fig


@app.callback(
    Output("market-price","figure"),
    Output("pnl-graph","figure"),
    Input("run-btn","n_clicks"),
    Input("bot-selector","value"),
    State("price-input","value"),
    State("spread-threshold-input","value"),
    State("window-input","value"),
    State("long-window-input","value"),
    State("short-window-input","value"),
    State("ticktime-input","value"),
    State("trailstop-input","value"),
    State("tick-input","value"),
    State("drift-input","value"),
    State("vol-input","value"),
    State("fix-seed-toggle","value"),
    State("seed-input", "value"),
    State("minhold-tick-input", "value"),
    State("proximity-sigma-input", "value"),
    State("stop-loss-input", "value")

    )
def update(n_clicks,selected_bots,price,spread_threshold,new_window,new_long_window,new_short_window,time_per_tick,trailstop,ticks,drift,vol,fix_seed_toggle,seed_input,minHoldTime, proxSigma,stopLoss):
    global dfMarket, dfTrades, window,long_window,short_window
    ctx=callback_context
    triggered_id=ctx.triggered[0]["prop_id"].split(".")[0]
    local_dfMarket = dfMarket.copy()
    new_trades=dfTrades.copy()
    if triggered_id == "run-btn" and n_clicks:
        # rewrite config.txt w/ parameters
        with open("../config.txt","w") as f:
            f.write(f"SPREAD_THRESHOLD={spread_threshold}\n")
            f.write(f"TRAIL_STOP_PCT={trailstop/100}\n")
            f.write(f"MEAN_REVERSION_WINDOW={new_window}\n")
            f.write(f"SHORT_WINDOW={new_short_window}\n")
            f.write(f"LONG_WINDOW={new_long_window}\n")
            f.write(f"TIME_PER_TICK_MS={time_per_tick}\n")
            f.write(f"INITIAL_PRICE={price}\n")
            f.write(f"DEFAULT_DRIFT={drift}\n")
            f.write(f"DEFAULT_VOLATILITY={vol}\n")
            f.write(f"MINHOLDTIME_TICK={minHoldTime}\n")
            f.write(f"PROXIMITY_SIGMA={proxSigma}\n")
            f.write(f"STOP_LOSS_PCT={stopLoss/100}\n")
        window=new_window
        new_short_window=short_window
        new_long_window=long_window
        # run simulation from project root 
        PROJECT_ROOT = os.path.abspath(os.path.join(BASE_DIR, os.pardir))
        cmd = ["./sim","-t",str(ticks)]
        if "fixed" in fix_seed_toggle:
            cmd.extend(["-s",str(seed_input)])
        subprocess.run(cmd,cwd=PROJECT_ROOT)

        #reload csvs
        local_dfMarket= load_market_data()
        new_trades = load_trades()
        dfMarket = local_dfMarket
        dfTrades = new_trades
    
    figMarket=make_base_fig(local_dfMarket)
    #update bots
    for bot in selected_bots:
        df=new_trades[bot]  
        if(bot=="maco"):
            figMarket=fig_add_crossing_SMAs(figMarket)
        if(bot=="mrb"):
            figMarket=fig_add_bollinger(figMarket)
        if df.empty:
            continue
        long_trades = df[df['direction'] == "LONG"]
        short_trades = df[df['direction'] == "SHORT"]

        figMarket.add_trace(go.Scatter(
            x=long_trades['entry_time'],
            y=long_trades['buy_price'],
            mode='markers',
            name=f'{bot} Long Entry',
            marker=dict(symbol='triangle-up', color='green', size=8)
        ))

        figMarket.add_trace(go.Scatter(
            x=short_trades['entry_time'],
            y=short_trades['buy_price'],
            mode='markers',
            name=f'{bot} Short Entry',
            marker=dict(symbol='triangle-down', color='red', size=8)
        ))

        figMarket.add_trace(go.Scatter(
            x=df['exit_time'],
            y=df['sell_price'],
            mode='markers',
            name=f'{bot} Exit',
            marker=dict(symbol='x', color='black', size=6)
        ))
    figMarket.update_layout(
    title="Market Price + bot Signals",
    xaxis_title="Time (ms)",
    yaxis_title="Price",
    legend=dict(orientation='h', yanchor='bottom', y=1.02, xanchor='right', x=1),
    height=600,
    template='plotly_white'
    )

    figPnL = fill_graph_pnl()

    return figMarket, figPnL

@app.callback(
    Output("tab-content", "children"),
    Input("tabs", "value")
)
def render_tab(tab):
    if tab == "simulation":
        return simulation_tab_layout
    elif tab == "sweep":
        return sweep_tab_layout()

def build_sweep_results():
    if not os.path.exists("sweep_results.csv"):
        return html.Div("No sweep results found.")

    try:
        df = pd.read_csv("sweep_results.csv")
        bots = df["Bot"].unique()
    except Exception as e:
        return html.Div([html.P("Failed to load sweep results:"), html.Pre(str(e))])

    return html.Div([
        html.H3("Sweep Results Explorer"),
        html.Label("Select Bot:"),
        dcc.Dropdown(
            id="sweep-bot-select",
            options=[{"label": b, "value": b} for b in bots],
            value=bots[0],
            style={"width": "300px"}
        ),
        dash_table.DataTable(
            id="sweep-table",
            page_size=10,
            sort_action="native",
            filter_action="native",
            style_table={"overflowX": "auto"},
            style_cell={"textAlign": "center"},
            style_header={"fontWeight": "bold"}
        ),
        dcc.Graph(id="sweep-heatmap", style={"marginTop": "40px"})
    ])
@app.callback(
    Output("sweep-results", "children"),
    Input("launch-sweep", "n_clicks"),
    State("vol-start", "value"),
    State("vol-stop", "value"),
    State("vol-step", "value"),
    State("spread-start", "value"),
    State("spread-stop", "value"),
    State("spread-step", "value"),
    State("sweep-seed", "value"),
    prevent_initial_call=True
)
def launch_sweep(n, vol_start, vol_stop, vol_step, spread_start, spread_stop, spread_step, seed):
    if seed is None or seed == "":
        seed = 42

    try:
        cmd = [
            "python3", "sweeper.py",
            "--vol-start", str(vol_start),
            "--vol-stop", str(vol_stop),
            "--vol-step", str(vol_step),
            "--spread-start", str(spread_start),
            "--spread-stop", str(spread_stop),
            "--spread-step", str(spread_step)
        ]
        if seed is not None:
            cmd += ["--seed", str(seed)]
        subprocess.run(cmd, check=True)
    except Exception as e:
        return f"Error : {str(e)}"
    return build_sweep_results()



@app.callback(
        Output("sweep-table","data"),
        Output("sweep-table","columns"),
        Input("sweep-bot-select","value")
)
def update_sweep_table(bot_name):
    df=pd.read_csv("sweep_results.csv")
    df=df[df["Bot"]==bot_name].copy()
    df=df.drop(columns=["Bot"])
    df=df.sort_values("Sharpe",ascending=False)
    columns=[{"name":col,"id":col} for col in df.columns]
    return df.to_dict("records"), [{"name":i,"id":i} for i in df.columns]


if __name__ == "__main__":
    app.run(debug=True)