#include "ReportGenerator.h"

void ReportGenerator::generate(const std::string& filename,
                               const std::vector<RoundStats>& mh,
                               const std::vector<RoundStats>& le,
                               int totalNodes) {
    std::ofstream f(filename);
    if (!f.is_open()) return;

    // Build JSON arrays for chart data
    auto buildArray = [](const std::vector<RoundStats>& stats, auto getter) {
        std::ostringstream ss;
        ss << "[";
        for (size_t i = 0; i < stats.size(); i++) {
            if (i > 0) ss << ",";
            ss << getter(stats[i]);
        }
        ss << "]";
        return ss.str();
    };

    std::string mhRounds = buildArray(mh, [](const RoundStats& r){ return r.round; });
    std::string leRounds = buildArray(le, [](const RoundStats& r){ return r.round; });
    std::string mhAlive  = buildArray(mh, [](const RoundStats& r){ return r.aliveNodes; });
    std::string leAlive  = buildArray(le, [](const RoundStats& r){ return r.aliveNodes; });
    std::string mhEnergy = buildArray(mh, [](const RoundStats& r){ return r.totalEnergyRemaining; });
    std::string leEnergy = buildArray(le, [](const RoundStats& r){ return r.totalEnergyRemaining; });
    std::string mhMsgs   = buildArray(mh, [](const RoundStats& r){ return r.messagesDelivered; });
    std::string leMsgs   = buildArray(le, [](const RoundStats& r){ return r.messagesDelivered; });
    std::string mhHops   = buildArray(mh, [](const RoundStats& r){ return r.avgHops; });
    std::string leHops   = buildArray(le, [](const RoundStats& r){ return r.avgHops; });

    // Compute totals for summary cards
    int mhTotalMsgs = 0, leTotalMsgs = 0;
    int mhFND = -1, leFND = -1;
    for (auto& s : mh) { mhTotalMsgs += s.messagesDelivered; if (s.deadNodes > 0 && mhFND == -1) mhFND = s.round; }
    for (auto& s : le) { leTotalMsgs += s.messagesDelivered; if (s.deadNodes > 0 && leFND == -1) leFND = s.round; }

    f << R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>WSN Simulator — Protocol Comparison Report</title>
<script src="https://cdn.jsdelivr.net/npm/chart.js@4.4.4/dist/chart.umd.min.js"></script>
<link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700&display=swap" rel="stylesheet">
<style>
  :root {
    --bg: #0f1117; --surface: #1a1d27; --surface2: #242836;
    --border: #2e3348; --text: #e4e6f0; --text-dim: #8b8fa3;
    --blue: #5b8af5; --purple: #a78bfa; --green: #34d399;
    --orange: #fb923c; --red: #f87171; --cyan: #22d3ee;
    --gradient1: linear-gradient(135deg, #5b8af5 0%, #a78bfa 100%);
    --gradient2: linear-gradient(135deg, #34d399 0%, #22d3ee 100%);
  }
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    font-family:'Inter',sans-serif; background:var(--bg); color:var(--text);
    min-height:100vh; padding:2rem;
  }
  .header {
    text-align:center; margin-bottom:3rem; padding:2.5rem;
    background:var(--surface); border-radius:20px; border:1px solid var(--border);
    position:relative; overflow:hidden;
  }
  .header::before {
    content:''; position:absolute; top:-50%; left:-50%; width:200%; height:200%;
    background:radial-gradient(circle at 30% 50%, rgba(91,138,245,0.08) 0%, transparent 50%),
               radial-gradient(circle at 70% 50%, rgba(167,139,250,0.08) 0%, transparent 50%);
  }
  .header h1 { font-size:2.2rem; font-weight:700; position:relative; }
  .header h1 span { background:var(--gradient1); -webkit-background-clip:text; -webkit-text-fill-color:transparent; }
  .header p { color:var(--text-dim); margin-top:0.5rem; position:relative; font-size:1rem; }

  .cards { display:grid; grid-template-columns:repeat(auto-fit,minmax(200px,1fr)); gap:1.2rem; margin-bottom:2.5rem; }
  .card {
    background:var(--surface); border:1px solid var(--border); border-radius:16px;
    padding:1.5rem; text-align:center; transition:transform 0.2s, box-shadow 0.2s;
  }
  .card:hover { transform:translateY(-3px); box-shadow:0 8px 30px rgba(0,0,0,0.3); }
  .card .label { font-size:0.75rem; text-transform:uppercase; letter-spacing:1.5px; color:var(--text-dim); margin-bottom:0.5rem; }
  .card .value { font-size:2rem; font-weight:700; }
  .card .sub { font-size:0.8rem; color:var(--text-dim); margin-top:0.3rem; }
  .card.blue .value { color:var(--blue); }
  .card.purple .value { color:var(--purple); }
  .card.green .value { color:var(--green); }
  .card.orange .value { color:var(--orange); }
  .card.cyan .value { color:var(--cyan); }
  .card.red .value { color:var(--red); }

  .charts { display:grid; grid-template-columns:repeat(auto-fit,minmax(500px,1fr)); gap:1.5rem; margin-bottom:2.5rem; }
  .chart-box {
    background:var(--surface); border:1px solid var(--border); border-radius:16px;
    padding:1.5rem;
  }
  .chart-box h3 { font-size:1rem; font-weight:600; margin-bottom:1rem; color:var(--text-dim); }
  .chart-box canvas { width:100% !important; }

  .table-section {
    background:var(--surface); border:1px solid var(--border); border-radius:16px;
    padding:1.5rem; overflow-x:auto;
  }
  .table-section h3 { font-size:1rem; font-weight:600; margin-bottom:1rem; color:var(--text-dim); }
  table { width:100%; border-collapse:collapse; font-size:0.85rem; }
  th { padding:0.7rem 1rem; text-align:left; border-bottom:2px solid var(--border); color:var(--text-dim); font-weight:600; text-transform:uppercase; font-size:0.7rem; letter-spacing:1px; }
  td { padding:0.6rem 1rem; border-bottom:1px solid var(--border); }
  tr:hover td { background:var(--surface2); }
  .tag { display:inline-block; padding:2px 8px; border-radius:6px; font-size:0.75rem; font-weight:600; }
  .tag-mh { background:rgba(91,138,245,0.15); color:var(--blue); }
  .tag-le { background:rgba(167,139,250,0.15); color:var(--purple); }

  .footer { text-align:center; padding:2rem; color:var(--text-dim); font-size:0.8rem; }

  @media(max-width:600px) {
    body { padding:1rem; }
    .charts { grid-template-columns:1fr; }
    .header h1 { font-size:1.5rem; }
  }
</style>
</head>
<body>

<div class="header">
  <h1>🛜 <span>WSN Simulator</span> — Protocol Comparison</h1>
  <p>Multi-Hop (BFS) vs LEACH (Cluster-Based) · )" << totalNodes << R"( Sensor Nodes</p>
</div>

<div class="cards">
  <div class="card blue">
    <div class="label">Multi-Hop Messages</div>
    <div class="value">)" << mhTotalMsgs << R"(</div>
    <div class="sub">Total delivered to sink</div>
  </div>
  <div class="card purple">
    <div class="label">LEACH Messages</div>
    <div class="value">)" << leTotalMsgs << R"(</div>
    <div class="sub">Aggregated to sink</div>
  </div>
  <div class="card green">
    <div class="label">Multi-Hop FND</div>
    <div class="value">R)" << mhFND << R"(</div>
    <div class="sub">First Node Death</div>
  </div>
  <div class="card orange">
    <div class="label">LEACH FND</div>
    <div class="value">R)" << leFND << R"(</div>
    <div class="sub">First Node Death</div>
  </div>
  <div class="card cyan">
    <div class="label">MH Rounds</div>
    <div class="value">)" << mh.size() << R"(</div>
    <div class="sub">Simulation length</div>
  </div>
  <div class="card red">
    <div class="label">LEACH Rounds</div>
    <div class="value">)" << le.size() << R"(</div>
    <div class="sub">Until all dead</div>
  </div>
</div>

<div class="charts">
  <div class="chart-box"><h3>📡 Alive Nodes per Round</h3><canvas id="aliveChart"></canvas></div>
  <div class="chart-box"><h3>🔋 Remaining Energy per Round</h3><canvas id="energyChart"></canvas></div>
  <div class="chart-box"><h3>📨 Messages Delivered per Round</h3><canvas id="msgsChart"></canvas></div>
  <div class="chart-box"><h3>🔄 Average Hops per Round</h3><canvas id="hopsChart"></canvas></div>
</div>

<div class="table-section">
  <h3>📊 Side-by-Side Round Data</h3>
  <table>
    <thead>
      <tr>
        <th>Round</th>
        <th>Protocol</th>
        <th>Alive</th>
        <th>Dead</th>
        <th>Energy Left</th>
        <th>Messages</th>
        <th>Avg Hops</th>
      </tr>
    </thead>
    <tbody id="tableBody"></tbody>
  </table>
</div>

<div class="footer">
  Generated by WSN Simulator v3.0 · Multi-Hop vs LEACH Protocol Comparison
</div>

<script>
const mhR=)" << mhRounds << R"(, leR=)" << leRounds << R"(;
const mhA=)" << mhAlive << R"(, leA=)" << leAlive << R"(;
const mhE=)" << mhEnergy << R"(, leE=)" << leEnergy << R"(;
const mhM=)" << mhMsgs << R"(, leM=)" << leMsgs << R"(;
const mhH=)" << mhHops << R"(, leH=)" << leHops << R"(;

const allRounds = [...new Set([...mhR,...leR])].sort((a,b)=>a-b);

Chart.defaults.color='#8b8fa3';
Chart.defaults.borderColor='#2e3348';
Chart.defaults.font.family='Inter';

function makeChart(id, label, mhData, leData, fill=false) {
  new Chart(document.getElementById(id), {
    type:'line',
    data:{
      labels:allRounds,
      datasets:[
        { label:'Multi-Hop', data:mhR.map((r,i)=>({x:r,y:mhData[i]})),
          borderColor:'#5b8af5', backgroundColor:'rgba(91,138,245,0.1)',
          borderWidth:2.5, pointRadius:4, pointHoverRadius:6, tension:0.3, fill },
        { label:'LEACH', data:leR.map((r,i)=>({x:r,y:leData[i]})),
          borderColor:'#a78bfa', backgroundColor:'rgba(167,139,250,0.1)',
          borderWidth:2.5, pointRadius:4, pointHoverRadius:6, tension:0.3, fill }
      ]
    },
    options:{
      responsive:true, interaction:{mode:'index',intersect:false},
      plugins:{legend:{labels:{usePointStyle:true,padding:20}}},
      scales:{
        x:{title:{display:true,text:'Round',font:{weight:'600'}},grid:{display:false}},
        y:{title:{display:true,text:label,font:{weight:'600'}},beginAtZero:true}
      }
    }
  });
}

makeChart('aliveChart','Alive Nodes',mhA,leA,true);
makeChart('energyChart','Energy (J)',mhE,leE,true);
makeChart('msgsChart','Messages',mhM,leM);
makeChart('hopsChart','Avg Hops',mhH,leH);

// Build table
const tb=document.getElementById('tableBody');
const maxR=Math.max(mhR.length,leR.length);
for(let i=0;i<maxR;i++){
  if(i<mhR.length){
    tb.innerHTML+=`<tr><td>${mhR[i]}</td><td><span class="tag tag-mh">Multi-Hop</span></td><td>${mhA[i]}</td><td>${)" << totalNodes << R"(-mhA[i]}</td><td>${mhE[i].toFixed(2)}</td><td>${mhM[i]}</td><td>${mhH[i].toFixed(1)}</td></tr>`;
  }
  if(i<leR.length){
    tb.innerHTML+=`<tr><td>${leR[i]}</td><td><span class="tag tag-le">LEACH</span></td><td>${leA[i]}</td><td>${)" << totalNodes << R"(-leA[i]}</td><td>${leE[i].toFixed(2)}</td><td>${leM[i]}</td><td>${leH[i].toFixed(1)}</td></tr>`;
  }
}
</script>
</body>
</html>)";

    f.close();
    std::cout << "[Report] Visual report generated: " << filename << "\n";
}
