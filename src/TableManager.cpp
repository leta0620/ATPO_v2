// cost part implementation
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <climits>

#include "TableManager.h"

using namespace std;

// Static member definition: routing length perturbation switch (default = on)
int TableManager::routingLengthEnable = 1;

// Utility: return the first non-"d" symbol inside a group.
// If the group contains only "d", return empty string.
static std::string ExtractGroupName(const Group& g) {
    for (const auto& du : g.GetDeviceUnits()) {
        const std::string& s = du.GetSymbol();
        if (!s.empty() && s != "d") return s;
    }
    return {};
}

// Utility: count how many units of each symbol appear inside a group (excluding "d").
static std::unordered_map<std::string, int> CountNameInGroup(const Group& g) {
    std::unordered_map<std::string, int> cnt;
    for (const auto& du : g.GetDeviceUnits()) {
        const std::string& s = du.GetSymbol();
        if (!s.empty() && s != "d") cnt[s] += 1;
    }
    return cnt;
}

bool CheckAllGroupMemberDummy(const Group& g) {
    for (const auto& du : g.GetDeviceUnits()) {
        const std::string& s = du.GetSymbol();
        if (!s.empty() && s != "d") return false;
    }
    return true;
}

// =======================
// Constructor & basic operations
// =======================

//TableManager::TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist)
//{
//    this->groupSize = groupSize;
//    this->rowSize = rowSize;
//    this->colSize = colSize;
//    this->netlist = netlist;
//
//	// set cost enum list to default (all cost types), use for loop
//    for (int i = static_cast<int>(CostEnum::ccCost); i <= static_cast<int>(CostEnum::hierCCost); i++) {
//        costEnumList.push_back(static_cast<CostEnum>(i));
//	}
//    InitializeTable();
//}

TableManager::TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist, std::vector<CostEnum> costEnumList)
{
    this->groupSize = groupSize;
    this->rowSize = rowSize;
    this->colSize = colSize;
    this->netlist = netlist;
    this->costEnumList = costEnumList;
    InitializeTable();
}

void TableManager::InitializeTable()
{
    table.resize(rowSize, std::vector<Group>(colSize));
}

// =======================
// Output part
// =======================

std::vector<std::string> TableManager::GetTableStringFormat()
{
    std::vector<std::string> tableStrings;

    for (auto& row : table)
    {
        std::string rowString;
        for (auto& group : row)
        {
            for (const auto& deviceUnit : group.GetDeviceUnits())
            {
                rowString += deviceUnit.GetSymbol();
            }
        }
        tableStrings.push_back(rowString);
    }

    return tableStrings;
}

std::vector<std::string> TableManager::GetTableStringPattern()
{
    std::vector<std::string> tableStrings;

    for (size_t rowIndex = 0; rowIndex < table.size(); rowIndex++)
    {
        auto& row = table[rowIndex];
        std::string rowString;
        for (auto& group : row)
        {
            auto deviceUnits = group.GetDeviceUnits();
            for (size_t i = 0; i < deviceUnits.size(); i++)
            {
                //rowString += deviceUnits[i].GetSymbol();
                rowString += deviceUnits[i].GetInstName();

                if (i < deviceUnits.size() - 1)
                {
                    rowString += ", ";
                }
            }
            if (&group < &row.back())
            {
                rowString += ", ";
            }
        }
        tableStrings.push_back(rowString);
    }

    return tableStrings;
}

std::vector<std::string> TableManager::GetTableRotationFormat(bool leftS)
{
    std::vector<std::string> tableRotations;
    for (auto& row : table)
    {
        std::string rowRotation;
        for (auto& group : row)
        {
            for (auto& deviceUnit : group.GetDeviceUnits())
            {
                rowRotation += deviceUnit.GetStringRotation(leftS);
            }
        }
        tableRotations.push_back(rowRotation);
    }
    return tableRotations;
}

std::vector<std::string> TableManager::GetTableRotationPattern(bool leftS)
{
    std::vector<std::string> tableRotations;
    for (size_t rowIndex = 0; rowIndex < table.size(); rowIndex++)
    {
        auto& row = table[rowIndex];
        std::string rowRotation;
        for (auto& group : row)
        {
            auto deviceUnits = group.GetDeviceUnits();
            for (size_t i = 0; i < deviceUnits.size(); i++)
            {
                rowRotation += deviceUnits[i].GetStringRotation(leftS);
                if (i < deviceUnits.size() - 1)
                {
                    rowRotation += ", ";
                }
            }
            if (&group < &row.back())
            {
                rowRotation += ", ";
            }
        }
        tableRotations.push_back(rowRotation);
    }
    return tableRotations;
}

// =======================
// Cost main function
// =======================

std::unordered_map<CostEnum, double> TableManager::CalculateTableCost()
{
    this->costMap.clear();

    for (auto& cItem : this->costEnumList)
    {
        switch (cItem)
        {
        case CostEnum::ccCost:
            this->CalculateCCCost();
            break;
        case CostEnum::rCost:
            this->CalculateRCost();
            break;
        case CostEnum::cCost:
            this->CalculateCCost();
            break;
        case CostEnum::sperationCost:
            this->CalculateSpetationCost();
            break;
        case CostEnum::dummyCost:
            this->CalculateDummyCost();
            break;
        case CostEnum::routing_lengthCost:
            this->CalculateRoutinglength();
            break;
        case CostEnum::mildCost:
            this->CalculateMILDCost();
            break;
        case CostEnum::congestionCost:
            this->CalculateCongestionCost();
            break;
        case CostEnum::symmetryCost:
            this->CalculateSymmetryCost();
            break;
        case CostEnum::windowSizeCost:
            this->CalculateWindowSizeCost();
            break;
        case CostEnum::hierCongestionCost:
            // HierCongestion 暫時保留程式碼但不擾動 SA:寫 0 維持 costMap entry,
            // 函式定義在 line ~2172 仍然完整保留(SHARED-tree 版本)。
            // 要重啟,刪除下面 = 0 那行並還原 CalculateHierCongestionCost() 呼叫。
            //this->CalculateHierCongestionCost();
            costMap[CostEnum::hierCongestionCost] = 0.0;
            break;
        case CostEnum::hierCCost:
            this->CalculateHierCCost();
            break;
        default:
            break;
        }
    }
    //this->CalculateCCCost();
    //this->CalculateRCost();
    //this->CalculateCCost();
    //this->CalculateSpetationCost();
    //this->CalculateDummyCost();
    //this->CalculateRoutinglength(); // ← 新增
    //this->CalculateMILDCost();
    //this->CalculateCongestionCost();
    //this->CalculateHierCongestionCost();
    //this->CalculateHierCCost();
    return this->costMap;
}

// =======================
// Cost part implementation
// =======================

// ---- CC (Center Closure): compute �g for each device name and average their |�g| ----
void TableManager::CalculateCCCost() {
    using std::string;
    using std::vector;
    using std::unordered_map;

    const int R = rowSize;
    const int G = colSize; // group columns
    if (R <= 0 || G <= 0) {
        costMap[CostEnum::ccCost] = 0.0;
        return;
    }

    // -----------------------
    // y centering by rows
    // -----------------------
    const bool rows_even = (R % 2 == 0);
    const int  rk = R / 2;        // even: center between rk-1, rk
    const int  rm = (R - 1) / 2;  // odd : center at rm

    auto y_of = [&](int r)->double {
        if (rows_even) {
            // ...,-1.5,-0.5,+0.5,+1.5,...
            return (static_cast<double>(r) + 0.5) - static_cast<double>(rk);
        }
        else {
            // ...,-2,-1,0,+1,+2,...
            return static_cast<double>(r - rm);
        }
        };

    // per-type accumulators
    unordered_map<string, double>    sum_x;
    unordered_map<string, double>    sum_y;
    unordered_map<string, long long> cnt;

    // -----------------------
    // sweep each row
    // -----------------------
    for (int r = 0; r < R; ++r) {
        // 1) flatten this row into unit-cell token sequence
        vector<string> rowTok;
        rowTok.reserve(G * 8); // rough guess (groupSize often 8)

        for (int c = 0; c < G; ++c) {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units) {
                rowTok.push_back(du.GetSymbol());
            }
        }

        const int W = static_cast<int>(rowTok.size());
        if (W == 0) continue;

        // 2) x centering by W
        const bool even = (W % 2 == 0);
        const int  k = W / 2;        // even: center between k-1, k
        const int  m = (W - 1) / 2;  // odd : center at m

        const double y = y_of(r);

        for (int i = 0; i < W; ++i) {
            const string& name = rowTok[i];
            if (name.empty() || name == "d") continue; // ignore dummy

            double x;
            if (even) {
                // ..., -3,-2,-1, +1,+2,+3,...
                x = (i < k)
                    ? (static_cast<double>(i) - static_cast<double>(k))
                    : (static_cast<double>(i) - static_cast<double>(k) + 1.0);
            }
            else {
                // ..., -2,-1,0,+1,+2,...
                x = static_cast<double>(i - m);
            }

            sum_x[name] += x;
            sum_y[name] += y;
            cnt[name] += 1;
        }
    }

    // -----------------------
    // per-type mu_x, mu_y
    // CCx = avg(|mu_x|), CCy = avg(|mu_y|)
    // CC  = CCx + CCy
    // -----------------------
    double sum_abs_mux = 0.0;
    double sum_abs_muy = 0.0;
    int    typeNum = 0;

    for (const auto& kv : cnt) {
        const string& name = kv.first;
        long long n = kv.second;
        if (n <= 0) continue;

        double mu_x = sum_x[name] / static_cast<double>(n);
        double mu_y = sum_y[name] / static_cast<double>(n);

        sum_abs_mux += std::fabs(mu_x);
        sum_abs_muy += std::fabs(mu_y);
        ++typeNum;
    }

    double CCx = 0.0, CCy = 0.0;
    if (typeNum > 0) {
        CCx = sum_abs_mux / static_cast<double>(typeNum);
        CCy = sum_abs_muy / static_cast<double>(typeNum);
    }

    costMap[CostEnum::ccCost] = CCx + CCy;
}



void TableManager::CalculateRCost() {
    using std::string;
    using std::vector;
    using std::unordered_map;

    unordered_map<string, double>    sum_wdist;
    unordered_map<string, long long> sum_cnt;

    for (int r = 0; r < rowSize; ++r) {
        vector<string> rowTok;
        rowTok.reserve(colSize * 8);
        for (int c = 0; c < colSize; ++c) {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units) {
                rowTok.push_back(du.GetSymbol());
            }
        }

        const int W = static_cast<int>(rowTok.size());
        if (W == 0) continue;

        const bool even = (W % 2 == 0);
        const int  k = W / 2;
        const int  m = (W - 1) / 2;

        for (int i = 0; i < W; ++i) {
            const string& name = rowTok[i];
            if (name.empty() || name == "d") continue;


            double x;
            if (even) x = (i < k) ? (i - k) : (i - k + 1);
            else      x = i - m;

            double dist = std::fabs(x);
            sum_wdist[name] += dist;
            sum_cnt[name] += 1;
        }
    }


    std::vector<double> per_name_avg;
    per_name_avg.reserve(sum_wdist.size());
    for (const auto& kv : sum_wdist) {
        const string& name = kv.first;
        long long cnt = sum_cnt[name];
        if (cnt <= 0) continue;
        per_name_avg.push_back(kv.second / static_cast<double>(cnt));
    }

    double r_cost = 0.0;
    if (!per_name_avg.empty()) {
        double s = 0.0, s2 = 0.0;
        for (double v : per_name_avg) {
            s += v;
            s2 += v * v;
        }
        double n = static_cast<double>(per_name_avg.size());
        double mean = s / n;
        double var = (s2 / n) - (mean * mean);
        if (var < 0.0) var = 0.0;
        r_cost = std::sqrt(var);
    }

    costMap[CostEnum::rCost] = r_cost;
}

// ---- C (Column Closure): both sides from outside to inside; add penalty when new name appears ----
void TableManager::CalculateCCost()
{
    using std::string;
    using std::vector;
    using std::unordered_map;
    using std::unordered_set;

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::cCost] = 0.0; return; }

    auto is_dummy = [](const string& s) -> bool {
        return s.empty() || s == "d";
        };

    // ============================================================
    // [Part 1] Per-cell congestion (routing-based wire density)
    // ============================================================
    // Trunk positions: SA, SB, DA, DB evenly distributed across rows
    // Even R: floor(i * R / 4)       — e.g. R=8 -> 0,2,4,6
    // Odd  R: round(i * (R-1) / 3)   — e.g. R=7 -> 0,2,4,6
    auto trunkRow = [&](int i) -> int {
        if (R % 2 == 0)
            return i * R / 4;
        else
            return (int)std::round(i * (R - 1) / 3.0);
        };
    const int trunkSA = trunkRow(0);
    const int trunkSB = trunkRow(1);
    const int trunkDA = trunkRow(2);
    const int trunkDB = trunkRow(3);

    // Group signature per cell
    auto groupSig = [&](int r, int c) -> string {
        unordered_set<string> s;
        for (const auto& du : table[r][c].GetDeviceUnits())
            if (!is_dummy(du.GetSymbol())) s.insert(du.GetSymbol());
        vector<string> v(s.begin(), s.end());
        std::sort(v.begin(), v.end());
        string sig;
        for (const auto& x : v) sig += x;
        return sig;
        };

    vector<vector<string>> sig(R, vector<string>(C));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            sig[r][c] = groupSig(r, c);

    // Identify unique groups and assign trunk pairs
    unordered_set<string> allSigs;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            if (!sig[r][c].empty()) allSigs.insert(sig[r][c]);

    vector<string> groupList(allSigs.begin(), allSigs.end());
    std::sort(groupList.begin(), groupList.end());

    // groupList[0] -> SA, DA; groupList[1] -> SB, DB
    unordered_map<string, int> sigTrunkS, sigTrunkD;
    if (groupList.size() >= 1) { sigTrunkS[groupList[0]] = trunkSA; sigTrunkD[groupList[0]] = trunkDA; }
    if (groupList.size() >= 2) { sigTrunkS[groupList[1]] = trunkSB; sigTrunkD[groupList[1]] = trunkDB; }

    // Count non-dummy units per cell
    vector<vector<int>> cellUnitCount(R, vector<int>(C, 0));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            for (const auto& du : table[r][c].GetDeviceUnits())
                if (!is_dummy(du.GetSymbol())) cellUnitCount[r][c]++;

    // For each column, compute vertical line ranges, then count lines per cell
    vector<vector<double>> cellCongest(R, vector<double>(C, 0.0));

    for (int c = 0; c < C; ++c)
    {
        struct Line { int rMin; int rMax; };
        vector<Line> lines;

        for (const auto& gSig : groupList)
        {
            if (sigTrunkS.find(gSig) == sigTrunkS.end()) continue;

            vector<int> rows;
            for (int r = 0; r < R; ++r)
                if (sig[r][c] == gSig) rows.push_back(r);
            if (rows.empty()) continue;

            int sTrunk = sigTrunkS[gSig];
            int dTrunk = sigTrunkD[gSig];

            // S-line range
            int sMin = sTrunk, sMax = sTrunk;
            for (int r : rows) { sMin = std::min(sMin, r); sMax = std::max(sMax, r); }
            lines.push_back({ sMin, sMax });

            // D-line range
            int dMin = dTrunk, dMax = dTrunk;
            for (int r : rows) { dMin = std::min(dMin, r); dMax = std::max(dMax, r); }
            lines.push_back({ dMin, dMax });
        }

        for (int r = 0; r < R; ++r)
        {
            int n = cellUnitCount[r][c];
            if (n == 0) continue;
            int count = 0;
            for (const auto& l : lines)
                if (r >= l.rMin && r <= l.rMax) count++;
            cellCongest[r][c] = static_cast<double>(count) / n;
        }
    }

    // ============================================================
    // [Part 2] Lateral x congestion
    // ============================================================
    vector<vector<string>> grid(R);
    vector<vector<int>>    unitCell(R);
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            for (const auto& du : table[r][c].GetDeviceUnits())
            {
                grid[r].push_back(du.GetSymbol());
                unitCell[r].push_back(c);
            }

    const int W = (int)grid[0].size();
    if (W <= 0) { costMap[CostEnum::cCost] = 0.0; return; }
    for (int r = 1; r < R; ++r)
        if ((int)grid[r].size() != W) { costMap[CostEnum::cCost] = 0.0; return; }

    const double wH = 1.0, wV = 0.26;
    vector<vector<double>> H_local(R, vector<double>(W, 0.0));
    vector<vector<double>> V_local(R, vector<double>(W, 0.0));

    for (int r = 0; r < R; ++r)
        for (int i = 0; i < W - 1; ++i)
        {
            const string& a = grid[r][i];
            const string& b = grid[r][i + 1];
            if (is_dummy(a) || is_dummy(b)) continue;
            if (a != b)
            {
                H_local[r][i] += cellCongest[r][unitCell[r][i]];
                H_local[r][i + 1] += cellCongest[r][unitCell[r][i + 1]];
            }
        }

    for (int i = 0; i < W; ++i)
        for (int r = 0; r < R - 1; ++r)
        {
            const string& a = grid[r][i];
            const string& b = grid[r + 1][i];
            if (is_dummy(a) || is_dummy(b)) continue;
            if (a != b)
            {
                V_local[r][i] += cellCongest[r][unitCell[r][i]];
                V_local[r + 1][i] += cellCongest[r + 1][unitCell[r + 1][i]];
            }
        }

    unordered_map<string, double> type_row_sum, type_col_sum;
    unordered_map<string, long long> type_cnt;
    for (int r = 0; r < R; ++r)
        for (int i = 0; i < W; ++i)
        {
            const string& t = grid[r][i];
            if (is_dummy(t)) continue;
            type_row_sum[t] += H_local[r][i];
            type_col_sum[t] += V_local[r][i];
            type_cnt[t]++;
        }

    double sum_avg_row = 0.0, sum_avg_col = 0.0;
    int type_num = 0;
    for (const auto& kv : type_cnt)
    {
        const string& t = kv.first;
        long long cnt = kv.second;
        if (cnt <= 0) continue;
        sum_avg_row += type_row_sum[t] / cnt;
        sum_avg_col += type_col_sum[t] / cnt;
        type_num++;
    }

    const double C_row = type_num > 0 ? sum_avg_row / type_num : 0.0;
    const double C_col = type_num > 0 ? sum_avg_col / type_num : 0.0;

    costMap[CostEnum::cCost] = wH * C_row + wV * C_col;
}



void TableManager::CalculateSpetationCost() {

    struct Pt { int r, x; }; // r = row index, x = unit index within the row
    std::unordered_map<std::string, std::vector<Pt>> cells;

    // Flatten into unit-cells (ignore "d")
    for (int r = 0; r < rowSize; ++r) {
        int x = 0;
        for (int c = 0; c < colSize; ++c) {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units) {
                const std::string& s = du.GetSymbol();
                if (!s.empty() && s != "d") cells[s].push_back(Pt{ r, x });
                x += 1; // index always increases regardless of symbol
            }
        }
    }

    if (cells.size() < 2) {
        costMap[CostEnum::sperationCost] = 0.0;
        return;
    }

    const double rho_u = 0.5;
    const double l_r = 1.0;
    const double l_c = 1.0;

    auto rho_of = [&](const Pt& a, const Pt& b)->double {
        const double dy = (a.r - b.r) * l_r;
        const double dx = (a.x - b.x) * l_c;
        const double d = std::sqrt(dy * dy + dx * dx);
        return std::pow(rho_u, d);
        };

    // Compute X_k for each symbol
    std::unordered_map<std::string, double> X;
    for (const auto& kv : cells) {
        const auto& pts = kv.second;
        const int n = static_cast<int>(pts.size());

        double inner = 0.0;
        for (int a = 0; a < n; ++a)
            for (int b = a + 1; b < n; ++b)
                inner += rho_of(pts[a], pts[b]);

        X[kv.first] = static_cast<double>(n) + 2.0 * inner;
        if (X[kv.first] <= 0.0) X[kv.first] = static_cast<double>(n);
    }

    // Compute �U �m_ij across all pairs
    std::vector<std::string> names;
    names.reserve(cells.size());
    for (const auto& kv : cells) names.push_back(kv.first);

    double total_sigma = 0.0;

    for (size_t i = 0; i + 1 < names.size(); ++i) {
        for (size_t j = i + 1; j < names.size(); ++j) {

            const auto& A = names[i];
            const auto& B = names[j];
            const auto& pA = cells[A];
            const auto& pB = cells[B];

            double numer = 0.0;
            for (const auto& a : pA)
                for (const auto& b : pB)
                    numer += rho_of(a, b);

            const double denom = std::sqrt(X[A] * X[B]);
            if (denom > 0.0) total_sigma += numer / denom;
        }
    }

    constexpr double kEps = 1e-12;           // Avoid delete 0
    double sigma = total_sigma;

    if (sigma < kEps) sigma = kEps;          // clamp
    costMap[CostEnum::sperationCost] = 1.0 / sigma;
}

void TableManager::CalculateRoutinglength()
{
    // Routing length = SHARED-tree per-net wire estimator (GROUP LEVEL).
    // See htree_shared_method memory note for full algorithm description.
    using std::string;
    using std::vector;

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::routing_lengthCost] = 0.0; return; }

    auto is_dummy = [](const string& s) { return s.empty() || s == "d"; };

    auto groupSig = [&](int r, int c) -> string {
        std::unordered_set<string> s;
        for (const auto& du : table[r][c].GetDeviceUnits())
            if (!is_dummy(du.GetSymbol())) s.insert(du.GetSymbol());
        vector<string> v(s.begin(), s.end());
        std::sort(v.begin(), v.end());
        string sig;
        for (const auto& x : v) sig += x;
        return sig;
        };

    vector<vector<string>> sig(R, vector<string>(C));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            sig[r][c] = groupSig(r, c);

    auto isAllSameSig = [&](int rS, int rE, int cS, int cE) -> bool {
        string single; bool found = false;
        for (int r = rS; r < rE; ++r)
            for (int c = cS; c < cE; ++c) {
                if (sig[r][c].empty()) continue;
                if (!found) { single = sig[r][c]; found = true; }
                else if (sig[r][c] != single) return false;
            }
        return found;
        };
    int cnt_h = 0, cnt_v = 0;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c + 1 < C; ++c)
            if (isAllSameSig(r, r + 1, c, c + 2)) cnt_h++;
    for (int r = 0; r + 1 < R; ++r)
        for (int c = 0; c < C; ++c)
            if (isAllSameSig(r, r + 2, c, c + 1)) cnt_v++;
    const bool h_init = (cnt_h >= cnt_v);

    vector<std::pair<int, int>> steps;
    {
        int rb = 1, cb = 1; bool h = h_init;
        while (rb < R || cb < C) {
            if (h) cb = std::min(cb * 2, C); else rb = std::min(rb * 2, R);
            steps.push_back({ rb, cb });
            if (rb >= R && cb >= C) break;
            h = !h;
        }
        if (steps.empty()) steps.push_back({ R, C });
    }

    auto getPartition = [](int dim) -> vector<int> {
        if (dim <= 0) return {};
        if (dim <= 3) return { dim };
        if (dim % 3 == 0) return vector<int>(dim / 3, 3);
        if (dim % 3 == 2) {
            vector<int> p(1, 2);
            for (int i = 0; i < (dim - 2) / 3; ++i) p.push_back(3);
            return p;
        }
        vector<int> p(1, 2);
        for (int i = 0; i < (dim - 4) / 3; ++i) p.push_back(3);
        p.push_back(2);
        return p;
        };
    auto bucketize = [&](int pos, int dim, int B) -> int {
        if (B >= dim) return 0;
        if (B < 2)    return pos;
        if (dim % 2 == 0) return std::min(pos / B, std::max(dim / B - 1, 0));
        vector<int> part = getPartition(dim);
        int cum = 0;
        for (int i = 0; i < (int)part.size(); ++i) {
            cum += part[i];
            if (pos < cum) return i;
        }
        return (int)part.size() - 1;
        };

    struct Node { double y, x; std::set<string> nets; };
    vector<Node> nodes;
    nodes.reserve((size_t)R * (size_t)C);
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            if (!sig[r][c].empty())
                nodes.push_back({ (double)r, (double)c, { sig[r][c] } });

    struct Edge { int p; int c; };
    vector<Edge> edges;
    vector<int> current;
    current.reserve(nodes.size());
    for (int i = 0; i < (int)nodes.size(); ++i) current.push_back(i);

    for (auto& step : steps) {
        const int rB = step.first; const int cB = step.second;
        std::map<std::pair<int, int>, vector<int>> buckets;
        for (int idx : current) {
            int wr = bucketize((int)nodes[idx].y, R, rB);
            int wc = bucketize((int)nodes[idx].x, C, cB);
            buckets[{wr, wc}].push_back(idx);
        }
        vector<int> new_current;
        new_current.reserve(buckets.size());
        for (auto& kv : buckets) {
            auto& group = kv.second;
            if (group.size() == 1) { new_current.push_back(group[0]); continue; }
            double cy = 0.0, cx = 0.0;
            std::set<string> mn;
            for (int g : group) {
                cy += nodes[g].y; cx += nodes[g].x;
                for (const auto& n : nodes[g].nets) mn.insert(n);
            }
            cy /= (double)group.size();
            cx /= (double)group.size();
            int parentIdx = (int)nodes.size();
            nodes.push_back({ cy, cx, std::move(mn) });
            for (int g : group) edges.push_back({ parentIdx, g });
            new_current.push_back(parentIdx);
        }
        current = std::move(new_current);
    }

    double total = 0.0;
    for (const auto& e : edges) {
        const Node& parent = nodes[e.p];
        const Node& child = nodes[e.c];
        double dist = std::abs(parent.y - child.y) + std::abs(parent.x - child.x);
        total += dist * (double)child.nets.size();
    }
    costMap[CostEnum::routing_lengthCost] = total;
}

#if 0
void TableManager::CalculateRoutinglength_OLD_DeviceUnitLevel()
{
    using std::string;
    using std::vector;

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::routing_lengthCost] = 0.0; return; }

    auto is_dummy = [](const string& s) { return s.empty() || s == "d"; };
    auto normPairFn = [](char a, char b) -> string { return { std::min(a,b), std::max(a,b) }; };

    static const int NUM_SUB = 4;

    // Build pattern strings per cell
    vector<vector<string>> cellPat(R, vector<string>(C));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            string s;
            for (const auto& du : table[r][c].GetDeviceUnits())
                s += du.GetSymbol();
            cellPat[r][c] = s;
        }

    int patLen = (int)cellPat[0][0].size();
    if (patLen < 2) { costMap[CostEnum::routing_lengthCost] = 0.0; return; }
    int gapsPerCell = patLen - 1;

    auto netAtPos = [&](int r, int c, int gi) -> string {
        return normPairFn(cellPat[r][c][gi], cellPat[r][c][gi + 1]);
        };

    // Target nets: exclude same-pair where symbol is outer letter
    std::set<char> outerLetters;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            outerLetters.insert(cellPat[r][c][0]);

    std::set<string> targetNets;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int gi = 0; gi < gapsPerCell; gi++) {
                string net = netAtPos(r, c, gi);
                if (net[0] == net[1] && outerLetters.count(net[0])) continue;
                targetNets.insert(net);
            }

    int gridRows = R * NUM_SUB;
    int gridCols = C * gapsPerCell;

    // Per-cell sub-row / rep-gap
    vector<vector<std::map<string, int>>> cellSubRow(R, vector<std::map<string, int>>(C));
    vector<vector<std::map<string, int>>> cellRepGap(R, vector<std::map<string, int>>(C));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            int nextSub = 0;
            std::set<string> seen;
            for (int gi = 0; gi < gapsPerCell; gi++) {
                string net = netAtPos(r, c, gi);
                if (!targetNets.count(net)) continue;
                if (seen.insert(net).second) {
                    cellSubRow[r][c][net] = nextSub++;
                    cellRepGap[r][c][net] = gi;
                }
            }
        }

    // UnionFind
    struct UF {
        vector<int> p;
        UF() {}
        UF(int n) : p(n) { for (int i = 0; i < n; i++) p[i] = i; }
        int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
        void unite(int a, int b) { a = find(a); b = find(b); if (a != b) p[a] = b; }
        bool connected(int a, int b) { return find(a) == find(b); }
    };

    auto cid = [&](int r, int c) { return r * gridCols + c; };
    auto toGC = [&](int cell, int gi) { return cell * gapsPerCell + gi; };

    auto isSeed = [&](int sr, int gc) -> bool {
        int origRow = sr / NUM_SUB;
        int cellCol = gc / gapsPerCell;
        int gi = gc % gapsPerCell;
        int subIdx = sr % NUM_SUB;
        string net = netAtPos(origRow, cellCol, gi);
        if (!targetNets.count(net)) return false;
        auto it = cellSubRow[origRow][cellCol].find(net);
        if (it == cellSubRow[origRow][cellCol].end() || it->second != subIdx) return false;
        auto it2 = cellRepGap[origRow][cellCol].find(net);
        return it2 != cellRepGap[origRow][cellCol].end() && it2->second == gi;
        };

    int totalCells = gridRows * gridCols;
    std::map<string, UF> nameUF;
    for (auto& n : targetNets) nameUF[n] = UF(totalCells);

    vector<vector<int>> congestion(gridRows, vector<int>(gridCols, 0));
    auto addLayer = [&](const vector<vector<int>>& L) {
        for (int r = 0; r < gridRows; r++)
            for (int c = 0; c < gridCols; c++)
                congestion[r][c] += L[r][c];
        };

    auto connectH = [&](int sr, int gc1, int gc2, vector<vector<int>>& L, const string& name) {
        auto& uf = nameUF[name];
        int gMin = std::min(gc1, gc2), gMax = std::max(gc1, gc2);
        for (int g = gMin; g <= gMax; g++) {
            if (L[sr][g] == 0) L[sr][g] = 1;
            uf.unite(cid(sr, gc1), cid(sr, g));
        }
        };
    auto connectV = [&](int gc, int sr1, int sr2, vector<vector<int>>& L, const string& name) {
        auto& uf = nameUF[name];
        int sMin = std::min(sr1, sr2), sMax = std::max(sr1, sr2);
        for (int sr = sMin; sr <= sMax; sr++) {
            if (L[sr][gc] == 0) L[sr][gc] = 1;
            uf.unite(cid(sr1, gc), cid(sr, gc));
        }
        };
    auto connectLShape = [&](int sr1, int gc1, int sr2, int gc2,
        vector<vector<int>>& Lv, vector<vector<int>>& Lh, const string& name) {
            auto& uf = nameUF[name];
            int sMin = std::min(sr1, sr2), sMax = std::max(sr1, sr2);
            for (int sr = sMin; sr <= sMax; sr++) {
                if (Lv[sr][gc1] == 0) Lv[sr][gc1] = 1;
                uf.unite(cid(sr1, gc1), cid(sr, gc1));
            }
            int gMin = std::min(gc1, gc2), gMax = std::max(gc1, gc2);
            for (int g = gMin; g <= gMax; g++) {
                if (Lh[sr2][g] == 0) Lh[sr2][g] = 1;
                uf.unite(cid(sr1, gc1), cid(sr2, g));
            }
        };

    // LG (UF only, no congestion)
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int gi = 0; gi < gapsPerCell; gi++) {
                string net = netAtPos(r, c, gi);
                if (!targetNets.count(net)) continue;
                int gc = toGC(c, gi);
                int srBase = r * NUM_SUB;
                auto& uf = nameUF[net];
                for (int s = 1; s < NUM_SUB; s++)
                    uf.unite(cid(srBase, gc), cid(srBase + s, gc));
            }

    // LG1
    {
        vector<vector<int>> L(gridRows, vector<int>(gridCols, 0));
        for (int r = 0; r < R; r++)
            for (int c = 0; c < C; c++) {
                std::map<string, vector<int>> netGaps;
                for (int gi = 0; gi < gapsPerCell; gi++) {
                    string net = netAtPos(r, c, gi);
                    if (targetNets.count(net)) netGaps[net].push_back(gi);
                }
                for (auto& [net, gis] : netGaps) {
                    if (gis.size() < 2) continue;
                    auto it = cellSubRow[r][c].find(net);
                    if (it == cellSubRow[r][c].end()) continue;
                    int sr = r * NUM_SUB + it->second;
                    int gcFirst = toGC(c, gis.front());
                    int gcLast = toGC(c, gis.back());
                    auto& uf = nameUF[net];
                    for (int g = gcFirst; g <= gcLast; g++) {
                        if (L[sr][g] == 0) L[sr][g] = 1;
                        uf.unite(cid(sr, gcFirst), cid(sr, g));
                    }
                }
            }
        addLayer(L);
    }

    // L0 (2-row pair vertical)
    {
        vector<vector<int>> L(gridRows, vector<int>(gridCols, 0));
        for (int rPair = 0; rPair + 1 < R; rPair += 2)
            for (int c = 0; c < C; c++)
                for (auto& net : targetNets) {
                    auto& rep0 = cellRepGap[rPair][c];
                    auto& rep1 = cellRepGap[rPair + 1][c];
                    auto& sub0 = cellSubRow[rPair][c];
                    auto& sub1 = cellSubRow[rPair + 1][c];
                    if (!rep0.count(net) || !rep1.count(net)) continue;
                    if (rep0.at(net) != rep1.at(net) || sub0.at(net) != sub1.at(net)) continue;
                    int gi = rep0.at(net);
                    int gc = toGC(c, gi);
                    int subIdx = sub0.at(net);
                    int sr0 = rPair * NUM_SUB + subIdx;
                    int sr1 = (rPair + 1) * NUM_SUB + subIdx;
                    auto& uf = nameUF[net];
                    for (int sr = sr0; sr <= sr1; sr++) {
                        L[sr][gc] = 1;
                        uf.unite(cid(sr0, gc), cid(sr, gc));
                    }
                }
        addLayer(L);
    }

    // Expansion layers
    {
        struct Step { int rBlk, cBlk; bool horiz; };
        vector<Step> steps;
        int rb = 2, cb = 1;
        bool h = true;
        while (rb < R || cb < C) {
            if (h) cb = std::min(cb * 2, C);
            else   rb = std::min(rb * 2, R);
            steps.push_back({ rb, cb, h });
            if (rb >= R && cb >= C) break;
            h = !h;
        }
        if (steps.empty()) steps.push_back({ R, C, true });

        for (auto& [rBlk, cBlk, horiz] : steps) {
            vector<vector<int>> L(gridRows, vector<int>(gridCols, 0));
            bool anyChange = false;
            for (int rS = 0; rS < R; rS += rBlk) {
                int rE = std::min(rS + rBlk, R);
                for (int cS = 0; cS < C; cS += cBlk) {
                    int cE = std::min(cS + cBlk, C);
                    int srS = rS * NUM_SUB, srE = rE * NUM_SUB;
                    int gcS = cS * gapsPerCell, gcE = cE * gapsPerCell;
                    for (const string& name : targetNets) {
                        vector<std::pair<int, int>> seeds;
                        for (int sr = srS; sr < srE; sr++)
                            for (int gc = gcS; gc < gcE; gc++)
                                if (isSeed(sr, gc) &&
                                    netAtPos(sr / NUM_SUB, gc / gapsPerCell, gc % gapsPerCell) == name)
                                    seeds.push_back({ sr, gc });
                        if (seeds.size() < 2) continue;
                        auto& uf = nameUF[name];
                        if (horiz) {
                            std::map<int, vector<int>> srToGCs;
                            for (auto& [sr, gc] : seeds) srToGCs[sr].push_back(gc);
                            vector<std::pair<int, vector<int>>> sv(srToGCs.begin(), srToGCs.end());
                            for (auto& [sr, gcs] : sv) std::sort(gcs.begin(), gcs.end());
                            std::sort(sv.begin(), sv.end(), [](auto& a, auto& b) {
                                return (a.second.back() - a.second.front()) > (b.second.back() - b.second.front());
                                });
                            for (auto& [sr, gcs] : sv) {
                                for (int i = 1; i < (int)gcs.size(); i++) {
                                    if (!uf.connected(cid(sr, gcs[i - 1]), cid(sr, gcs[i]))) {
                                        connectH(sr, gcs[i - 1], gcs[i], L, name);
                                        anyChange = true;
                                    }
                                }
                            }
                        }
                        else {
                            std::map<int, vector<int>> gcToSRs;
                            for (auto& [sr, gc] : seeds) gcToSRs[gc].push_back(sr);
                            vector<std::pair<int, vector<int>>> sv(gcToSRs.begin(), gcToSRs.end());
                            for (auto& [gc, srs] : sv) std::sort(srs.begin(), srs.end());
                            std::sort(sv.begin(), sv.end(), [](auto& a, auto& b) {
                                return (a.second.back() - a.second.front()) > (b.second.back() - b.second.front());
                                });
                            for (auto& [gc, srs] : sv) {
                                for (int i = 1; i < (int)srs.size(); i++) {
                                    if (!uf.connected(cid(srs[i - 1], gc), cid(srs[i], gc))) {
                                        connectV(gc, srs[i - 1], srs[i], L, name);
                                        anyChange = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (anyChange) addLayer(L);
        }
    }

    // L-shape
    {
        vector<vector<int>> Lv(gridRows, vector<int>(gridCols, 0));
        vector<vector<int>> Lh(gridRows, vector<int>(gridCols, 0));
        for (const string& name : targetNets) {
            vector<std::pair<int, int>> seeds;
            for (int sr = 0; sr < gridRows; sr++)
                for (int gc = 0; gc < gridCols; gc++)
                    if (isSeed(sr, gc) &&
                        netAtPos(sr / NUM_SUB, gc / gapsPerCell, gc % gapsPerCell) == name)
                        seeds.push_back({ sr, gc });
            if (seeds.size() < 2) continue;
            auto& uf = nameUF[name];
            for (int i = 1; i < (int)seeds.size(); i++) {
                auto [s0, g0] = seeds[0];
                auto [si, gi] = seeds[i];
                if (!uf.connected(cid(s0, g0), cid(si, gi)))
                    connectLShape(s0, g0, si, gi, Lv, Lh, name);
            }
        }
        bool hasV = false, hasH = false;
        for (int r = 0; r < gridRows && !hasV; r++)
            for (int c = 0; c < gridCols && !hasV; c++) if (Lv[r][c] > 0) hasV = true;
        for (int r = 0; r < gridRows && !hasH; r++)
            for (int c = 0; c < gridCols && !hasH; c++) if (Lh[r][c] > 0) hasH = true;
        if (hasV) addLayer(Lv);
        if (hasH) addLayer(Lh);
    }

    // Total congestion sum
    long long total = 0;
    for (int r = 0; r < gridRows; r++)
        for (int c = 0; c < gridCols; c++)
            total += congestion[r][c];

    costMap[CostEnum::routing_lengthCost] = (double)total;
}
#endif // disabled old device-unit-level routing length


void TableManager::CalculateMILDCost()
{
    using std::string;
    using std::unordered_map;
    using std::vector;

    const double Lg = 1.0;

    // ----------------------------------------------------------
    // Compute w: total unit cells per row
    // ----------------------------------------------------------
    int w = 0;
    if (rowSize > 0)
        for (int c = 0; c < colSize; ++c)
            w += (int)table[0][c].GetDeviceUnits().size();

    if (w == 0)
    {
        costMap[CostEnum::mildCost] = 0.0;
        return;
    }

    // ----------------------------------------------------------
    // Step 1: For each device, collect 1/LOD of each unit cell
    // ----------------------------------------------------------
    unordered_map<string, double> sumInvLOD;
    unordered_map<string, int>    unitCount;

    for (int r = 0; r < rowSize; ++r)
    {
        int x = 0;                          // 0-based: x = 0 .. w-1
        for (int c = 0; c < colSize; ++c)
        {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units)
            {
                const string& sym = du.GetSymbol();
                if (!sym.empty() && sym != "d")
                {
                    double invLOD = 1.0 / (x + 0.5 * Lg)
                        + 1.0 / (w - 1 - x + 0.5 * Lg);
                    sumInvLOD[sym] += invLOD;
                    unitCount[sym]++;
                }
                ++x;
            }
        }
    }

    if (sumInvLOD.size() < 2)
    {
        costMap[CostEnum::mildCost] = 0.0;
        return;
    }

    // ----------------------------------------------------------
    // Step 2: mean 1/LOD per device
    // ----------------------------------------------------------
    unordered_map<string, double> meanInvLOD;
    for (const auto& kv : sumInvLOD)
        meanInvLOD[kv.first] = kv.second / unitCount[kv.first];

    // ----------------------------------------------------------
    // Step 3: MILD = Σ counterpart pairs |mean(1/LOD)_a - mean(1/LOD)_b|
    // Counterpart: same position in different group patterns
    //   BUT extract ONLY WITHIN the same half (left vs left, right vs right).
    //   Cross-half comparison would introduce spurious pairs when left and
    //   right halves use different (mirrored) cell patterns.
    // e.g. CAAC vs DBBD -> C↔D (pos 0,3), A↔B (pos 1,2)
    // ----------------------------------------------------------

    // Lambda: collect unique group patterns from a column range
    auto collectUniquePats = [&](int cStart, int cEnd) {
        vector<string> pats;
        std::set<string> seen;
        for (int r = 0; r < rowSize; ++r) {
            for (int c = cStart; c < cEnd; ++c) {
                string pat;
                for (const auto& du : table[r][c].GetDeviceUnits()) {
                    const string& s = du.GetSymbol();
                    if (!s.empty() && s != "d") pat += s;
                }
                if (!pat.empty() && seen.insert(pat).second)
                    pats.push_back(pat);
            }
        }
        return pats;
        };

    // Lambda: extract counterpart pairs from pairwise comparison within a group
    auto extractCounterparts = [](const vector<string>& pats,
        std::set<std::pair<string, string>>& cps) {
            for (int i = 0; i < (int)pats.size() - 1; ++i)
                for (int j = i + 1; j < (int)pats.size(); ++j) {
                    const string& pa = pats[i];
                    const string& pb = pats[j];
                    int len = std::min((int)pa.size(), (int)pb.size());
                    for (int k = 0; k < len; ++k) {
                        string sa(1, pa[k]), sb(1, pb[k]);
                        if (sa != sb) {
                            if (sa > sb) std::swap(sa, sb);
                            cps.insert({ sa, sb });
                        }
                    }
                }
        };

    const int halfCol = colSize / 2;
    vector<string> leftPats = collectUniquePats(0, halfCol);
    vector<string> rightPats = collectUniquePats(halfCol, colSize);

    std::set<std::pair<string, string>> counterparts;
    extractCounterparts(leftPats, counterparts);
    extractCounterparts(rightPats, counterparts);

    double mild = 0.0;
    for (const auto& [a, b] : counterparts) {
        if (meanInvLOD.count(a) && meanInvLOD.count(b))
            mild += std::abs(meanInvLOD[a] - meanInvLOD[b]);
    }

    costMap[CostEnum::mildCost] = mild;
    costMap[CostEnum::mildCost] = mild < 1e-12 ? 0.0 : mild;

}







// =======================
// Rule checks (currently always return true)
// =======================

// check colume rule(same type sequential)
bool TableManager::ColumnRuleCheck(int rowPlace, int colPlace, Group& group)
{
    int nowGroupTypeHash = group.GetTypeHash();

    // check above
    if (rowPlace - 1 >= 0)
    {
        if (!table[rowPlace - 1][colPlace].HasDummyUnit())
        {
            int aboveGroupTypeHash = table[rowPlace - 1][colPlace].GetTypeHash();
            if (nowGroupTypeHash != aboveGroupTypeHash)
            {
                return false;
            }
        }
    }

    // check below
    if (rowPlace + 1 < rowSize)
    {
        if (!table[rowPlace + 1][colPlace].HasDummyUnit())
        {
            int belowGroupTypeHash = table[rowPlace + 1][colPlace].GetTypeHash();
            if (nowGroupTypeHash != belowGroupTypeHash)
            {
                return false;
            }
        }
    }

    return true;
}
// check row rule(neighborhood group can link)
bool TableManager::RowRuleCheck(int rowPlace, int colPlace, Group& group)
{
    vector<DeviceUnit> groupDeviceUnits = group.GetDeviceUnits();

    // check left
    DeviceUnit firstDeviceUnit = groupDeviceUnits[0];
    if (colPlace - 1 >= 0)
    {
        Group leftGroup = table[rowPlace][colPlace - 1];
        pair<string, CellRotation> leftGroupLastWhoAndRotation = leftGroup.GetLastDeviceUnitWhoAndRotation();

        if (leftGroupLastWhoAndRotation.second == CellRotation::R0)
        {
            auto tmp = this->netlist.GetPinSLinkWho(leftGroupLastWhoAndRotation.first).first;

            if (this->netlist.GetPinSLinkWho(leftGroupLastWhoAndRotation.first).first != firstDeviceUnit.GetSymbol() && !(this->netlist.GetPinSLinkWho(leftGroupLastWhoAndRotation.first).first == "COMMON_SOURCE" && firstDeviceUnit.GetRotation() == CellRotation::MY && this->netlist.GetPinSLinkWho(firstDeviceUnit.GetSymbol()).first == "COMMON_SOURCE"))
            {
                return false;
            }
        }
        else if (leftGroupLastWhoAndRotation.second == CellRotation::MY)
        {
            auto tmp = this->netlist.GetPinSLinkWho(leftGroupLastWhoAndRotation.first).first;

            if (this->netlist.GetPinDLinkWho(leftGroupLastWhoAndRotation.first).first != firstDeviceUnit.GetSymbol())
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    // check right
    DeviceUnit lastDeviceUnit = groupDeviceUnits.back();
    if (colPlace + 1 < colSize)
    {
        Group rightGroup = table[rowPlace][colPlace + 1];
        pair<string, CellRotation> rightGroupFirstWhoAndRotation = rightGroup.GetFirstDeviceUnitWhoAndRotation();
        //if (rightGroupFirstWhoAndOuterPin.first != lastDeviceUnit.GetSymbol()) return false;

        if (rightGroupFirstWhoAndRotation.second == CellRotation::R0)
        {
            auto tmp = this->netlist.GetPinSLinkWho(rightGroupFirstWhoAndRotation.first).first;

            if (this->netlist.GetPinDLinkWho(rightGroupFirstWhoAndRotation.first).first != lastDeviceUnit.GetSymbol())
            {
                return false;
            }
        }
        else  if (rightGroupFirstWhoAndRotation.second == CellRotation::MY)
        {
            auto tmp = this->netlist.GetPinSLinkWho(rightGroupFirstWhoAndRotation.first).first;

            if (this->netlist.GetPinSLinkWho(rightGroupFirstWhoAndRotation.first).first != lastDeviceUnit.GetSymbol() && !(this->netlist.GetPinSLinkWho(rightGroupFirstWhoAndRotation.first).first == "COMMON_SOURCE" && lastDeviceUnit.GetRotation() == CellRotation::R0 && this->netlist.GetPinSLinkWho(lastDeviceUnit.GetSymbol()).first == "COMMON_SOURCE"))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

// =======================
// Placement operations (not implemented yet)
// =======================

bool TableManager::PlaceGroup(const Group& group, int& placedRow, int& placedCol)
{
    if (placedRow < 0 || placedRow >= this->rowSize || placedCol < 0 || placedCol >= this->colSize)
        return false;
    else
    {
        this->table[placedRow][placedCol] = group;
        return true;
    }

    //Group placedGroup = group;
    //if (this->ColumnRuleCheck(placedRow, placedCol, placedGroup) && this->RowRuleCheck(placedRow, placedCol, placedGroup))
    //{
    //	this->table[placedRow][placedCol] = placedGroup;
    //	return true;
    //}

    return false;
}

bool TableManager::SwapGroups(int row1, int col1, int row2, int col2)
{
    Group group1 = table[row1][col1];
    Group group2 = table[row2][col2];

    /*if (this->ColumnRuleCheck(row1, col1, group2) && this->RowRuleCheck(row1, col1, group2) &&
        this->ColumnRuleCheck(row2, col2, group1) && this->RowRuleCheck(row2, col2, group1))
    {
        this->table[row1][col1] = group2;
        this->table[row2][col2] = group1;
        return true;
    }*/

    auto SetGroupMemberWidth = [](Group& gDest, Group& gSource)
        {
            std::vector<DeviceUnit> sourceDeviceUnits = gSource.GetDeviceUnits();
            std::vector<DeviceUnit> destDeviceUnits = gDest.GetDeviceUnits();
            for (size_t i = 0; i < sourceDeviceUnits.size() && i < destDeviceUnits.size(); i++)
            {
                destDeviceUnits[i].SetWidth(sourceDeviceUnits[i].GetWidth());
            }
            gDest.SetDeviceUnits(destDeviceUnits);
        };

    if (this->CheckCanSwapGroups(row1, col1, row2, col2))
    {
        if (CheckAllGroupMemberDummy(group1))
        {
            SetGroupMemberWidth(group1, group2);
        }
        else if (CheckAllGroupMemberDummy(group2))
        {
            SetGroupMemberWidth(group2, group1);
        }

        this->table[row1][col1] = group2;
        this->table[row2][col2] = group1;

        return true;
    }

    return false;
}

bool TableManager::MoveGroup(int srcRow, int srcCol, int destRow, int destCol)
{
    Group srcGroup = table[srcRow][srcCol];
    if (this->ColumnRuleCheck(destRow, destCol, srcGroup) && this->RowRuleCheck(destRow, destCol, srcGroup))
    {
        this->table[destRow][destCol] = srcGroup;
        // clear source position
        this->table[srcRow][srcCol] = Group();
        return true;
    }

    return false;
}

// =======================
// Utility functions
// =======================

bool TableManager::EqualTableToSelf(TableManager& otherTable)
{
    std::vector<std::string> thisTableStrings = this->GetTableStringFormat();
    std::vector<std::string> otherTableStrings = otherTable.GetTableStringFormat();
    return thisTableStrings == otherTableStrings;
}

void TableManager::SetTableSize(int rowSize, int colSize)
{
    this->rowSize = rowSize;
    this->colSize = colSize;
    InitializeTable();
}

bool TableManager::CheckCanSwapGroups(int row1, int col1, int row2, int col2)
{
    if (row1 < 0 || row1 >= this->rowSize || col1 < 0 || col1 >= this->colSize ||
        row2 < 0 || row2 >= this->rowSize || col2 < 0 || col2 >= this->colSize)
        return false;

    Group group1 = table[row1][col1];
    Group group2 = table[row2][col2];

    bool allMemberDummy1 = true, allMemberDummy2 = true;

    allMemberDummy1 = CheckAllGroupMemberDummy(group1);
    allMemberDummy2 = CheckAllGroupMemberDummy(group2);

    if (allMemberDummy1 && allMemberDummy2)
    {
        return false;
    }

    if (allMemberDummy1 || allMemberDummy2)
    {
        return true;
    }

    if (this->ColumnRuleCheck(row1, col1, group2) && this->ColumnRuleCheck(row2, col2, group1))
    {
        return true;
    }
    return false;
}

bool TableManager::SwapColumns(int col1, int col2)
{
    if (col1 < 0 || col1 >= this->colSize || col2 < 0 || col2 >= this->colSize)
        return false;

    for (int r = 0; r < this->rowSize; r++)
    {
        Group group1 = table[r][col1];
        Group group2 = table[r][col2];
        table[r][col1] = group2;
        table[r][col2] = group1;
    }
    return true;
}

bool TableManager::SwapRows(int row1, int row2)
{
    if (row1 < 0 || row1 >= this->rowSize || row2 < 0 || row2 >= this->rowSize)
        return false;
    for (int c = 0; c < this->colSize; c++)
    {
        Group group1 = table[row1][c];
        Group group2 = table[row2][c];
        table[row1][c] = group2;
        table[row2][c] = group1;
    }
    return true;
}


std::vector<std::pair<std::string, double>> TableManager::GetCostNameAndCostValueString()
{
    std::vector< std::pair<std::string, double>> costNameAndValue;
    for (const auto& [costEnum, costValue] : this->costMap)
    {
        // HierCongestion 暫時停用 → 從輸出顯示中隱藏
        // 函式定義 (line ~2172) 跟 case (line ~227) 都保留,要恢復就把這幾行 // 拿掉
        if (costEnum == CostEnum::hierCongestionCost) continue;

        std::string costName;

        costName = GetCostName(costEnum);

        //     switch (costEnum)
        //     {
        //     case CostEnum::ccCost:
        //         costName = "System variation";
        //         break;
        //     case CostEnum::rCost:
        //         costName = "Routing Length";
        //         break;
        //     case CostEnum::cCost:
        //         costName = "Coupling capacitance";
        //         break;
        //     case CostEnum::sperationCost:
        //         costName = "Dispersion";
        //         break;
        //     case CostEnum::dummyCost:
        //         costName = "Dummy Penalty";
                 //break;
        //     default:
        //         costName = "Unknown Cost";
        //         break;
        //     }
        costNameAndValue.push_back(pair<string, double>(costName, costValue));
    }
    return costNameAndValue;
}

void TableManager::PrintTableToConsole()
{
    for (auto& row : table)
    {
        for (auto& group : row)
        {
            for (const auto& deviceUnit : group.GetDeviceUnits())
            {
                std::cout << deviceUnit.GetSymbol();
            }
        }
        std::cout << std::endl;
    }
}

bool TableManager::CheckAndFixDummyWidth()
{
    vector<vector<int>> dummyWidthTable;
    dummyWidthTable.reserve(this->colSize);
    //this->PrintTableToConsole();

    for (int c = 0; c < this->colSize; c++)
    {
        vector<int> dummyWidthsInCol;
        dummyWidthsInCol.resize(this->table[0][c].GetDeviceUnits().size());


        for (int r = 0; r < this->rowSize; r++)
        {
            Group& g = this->table[r][c];
            vector<DeviceUnit> deviceUnits = g.GetDeviceUnits();
            bool allWidthSave = true;

            for (int d = 0; d < deviceUnits.size(); d++)
            {
                if (deviceUnits[d].GetSymbol() != "d")
                {
                    dummyWidthsInCol[d] = deviceUnits[d].GetWidth();
                }
                else
                {
                    allWidthSave = false;
                }
            }

            if (allWidthSave)
            {
                dummyWidthTable.push_back(dummyWidthsInCol);
                break;
            }
        }
    }

    // fix dummy width
    for (int c = 0; c < this->colSize; c++)
    {
        for (int r = 0; r < this->rowSize; r++)
        {
            Group& g = this->table[r][c];
            vector<DeviceUnit> deviceUnits = g.GetDeviceUnits();
            for (int d = 0; d < deviceUnits.size(); d++)
            {
                if (deviceUnits[d].GetSymbol() == "d")
                {
                    deviceUnits[d].SetWidth(dummyWidthTable[c][d]);
                }
            }
            g.SetDeviceUnits(deviceUnits);
        }
    }



    return true;
}

vector<string> TableManager::GetTableStringPatternInRealDummyLength()
{
    std::vector<std::string> tableStrings;
    vector<int> rowInstNum;
    rowInstNum.resize(this->rowSize);
    int maxRowInstNum = 0;

    for (size_t rowIndex = 0; rowIndex < this->table.size(); rowIndex++)
    {
        string rowString;
        for (size_t groupIndex = 0; groupIndex < table[rowIndex].size(); groupIndex++)
        {
            for (size_t deviceIndex = 0; deviceIndex < table[rowIndex][groupIndex].GetDeviceUnits().size(); deviceIndex++)
            {
                DeviceUnit deviceUnit = table[rowIndex][groupIndex].GetDeviceUnits()[deviceIndex];

                vector<string> instNames = deviceUnit.GetPatternUseNameList();
                rowInstNum[rowIndex] += instNames.size();
                if (rowInstNum[rowIndex] > maxRowInstNum)
                {
                    maxRowInstNum = rowInstNum[rowIndex];
                }


                for (size_t i = 0; i < instNames.size(); i++)
                {
                    rowString += instNames[i];

                    if (i < instNames.size() - 1)
                    {
                        rowString += ", ";
                    }
                }

                if (deviceIndex < table[rowIndex][groupIndex].GetDeviceUnits().size() - 1)
                {
                    rowString += ", ";
                }
            }
            if (groupIndex < table[rowIndex].size() - 1)
            {
                rowString += ", ";
            }
        }
        tableStrings.push_back(rowString);
    }

    // fix word length
    for (size_t rowIndex = 0; rowIndex < this->table.size(); rowIndex++)
    {
        int needAddDummyNum = maxRowInstNum - rowInstNum[rowIndex];
        if (needAddDummyNum > 0)
        {
            string& rowString = tableStrings[rowIndex];
            for (int i = 0; i < needAddDummyNum; i++)
            {
                rowString += ", *";
            }
        }
    }

    return tableStrings;
}

vector<string> TableManager::GetTableRotationPatternInRealDummyLength(bool leftS)
{
    std::vector<std::string> tableRotations;
    vector<int> rowInstNum;
    rowInstNum.resize(this->rowSize);
    int maxRowInstNum = 0;

    for (size_t rowIndex = 0; rowIndex < this->table.size(); rowIndex++)
    {
        string rowString;
        for (size_t groupIndex = 0; groupIndex < table[rowIndex].size(); groupIndex++)
        {
            for (size_t deviceIndex = 0; deviceIndex < table[rowIndex][groupIndex].GetDeviceUnits().size(); deviceIndex++)
            {
                DeviceUnit deviceUnit = table[rowIndex][groupIndex].GetDeviceUnits()[deviceIndex];

                vector<string> rotations = deviceUnit.GetPatternUseRotationList();
                rowInstNum[rowIndex] += rotations.size();
                if (rowInstNum[rowIndex] > maxRowInstNum)
                {
                    maxRowInstNum = rowInstNum[rowIndex];
                }

                for (size_t i = 0; i < rotations.size(); i++)
                {
                    rowString += rotations[i];
                    if (i < rotations.size() - 1)
                    {
                        rowString += ", ";
                    }
                }

                if (deviceIndex < table[rowIndex][groupIndex].GetDeviceUnits().size() - 1)
                {
                    rowString += ", ";
                }
            }
            if (groupIndex < table[rowIndex].size() - 1)
            {
                rowString += ", ";
            }
        }
        tableRotations.push_back(rowString);
    }

    //fix word length
    for (size_t rowIndex = 0; rowIndex < this->table.size(); rowIndex++)
    {
        int needAddDummyNum = maxRowInstNum - rowInstNum[rowIndex];
        if (needAddDummyNum > 0)
        {
            string& rowString = tableRotations[rowIndex];
            for (int i = 0; i < needAddDummyNum; i++)
            {
                rowString += ", R0";
            }
        }

    }
    return tableRotations;
}

string TableManager::GetCostName(CostEnum costEnum)
{
    if (costEnum == CostEnum::ccCost)
    {
        return "System variation";
    }
    else if (costEnum == CostEnum::rCost)
    {
        return "Length Maching";
    }
    else if (costEnum == CostEnum::cCost)
    {
        return "Coupling capacitance";
    }
    else if (costEnum == CostEnum::sperationCost)
    {
        return "Dispersion";
    }
    else if (costEnum == CostEnum::dummyCost)
    {
        return "Dummy Penalty";
    }
    else if (costEnum == CostEnum::routing_lengthCost)
    {
        return "Routing Length";
    }

    else if (costEnum == CostEnum::mildCost)
    {
        return "LOD";
    }
    else if (costEnum == CostEnum::congestionCost)
    {
        return "Congestion";
    }
    else if (costEnum == CostEnum::hierCongestionCost)
    {
        return "Hierarchical Congestion";
    }
    else if (costEnum == CostEnum::hierCCost)
    {
        return "Hierarchical Coupling capacitance";
    }
    else if (costEnum == CostEnum::windowSizeCost)
    {
        return "Window Size Cost";
    }
    else if (costEnum == CostEnum::symmetryCost)
    {
        return "Symmetry Cost";
    }
    else
    {
        return "Unknown Cost";
    }
}


bool TableManager::BuildInterleavingTable()
{
    std::unordered_map<std::string, int> groupTypeCount;
    unordered_map<string, Group> groupTypeExample;

    for (auto& r : this->table)
    {
        for (auto& g : r)
        {
            if (g.CheckAllDummyUnit())
            {
                continue;
            }

            std::string sNS = g.GetSymbolNameSequence();
            // 產生反轉字串
            std::string rev = sNS;
            std::reverse(rev.begin(), rev.end());

            // 先尋找正向 key
            auto it = groupTypeCount.find(sNS);
            if (it != groupTypeCount.end())
            {
                it->second += 1;
            }
            else
            {
                // find reverse key
                auto it_rev = groupTypeCount.find(rev);
                if (it_rev != groupTypeCount.end())
                {
                    it_rev->second += 1;
                }
                else
                {
                    // add new key
                    groupTypeCount.emplace(sNS, 1);
                    groupTypeExample.emplace(sNS, g);
                }
            }
        }
    }

    // build interleaving table
    vector<vector<Group>> interleavingTable;
    vector<vector<Group>> dummyGroupTable;
    for (auto& item : groupTypeCount)
    {
        std::string sNS = item.first;
        int count = item.second;

        while (count > 0)
        {
            if (count > rowSize)
            {
                vector<Group> rowGroups(rowSize);
                for (int i = 0; i < rowSize; i++)
                {
                    rowGroups[i] = groupTypeExample[sNS];
                }
                interleavingTable.push_back(rowGroups);
                count -= rowSize;
            }
            else
            {
                int upDummy = (rowSize - count) / 2;
                int downDummy = rowSize - count - upDummy;

                int groupSize = groupTypeExample[sNS].GetDeviceUnits().size();

                Group dummyGroup;
                dummyGroup.BuildAllDummyGroup(groupSize);

                vector<Group> rowGroups;
                for (int i = 0; i < upDummy; i++)
                {
                    rowGroups.push_back(dummyGroup);
                }
                for (int i = 0; i < count; i++)
                {
                    rowGroups.push_back(groupTypeExample[sNS]);
                }
                for (int i = 0; i < downDummy; i++)
                {
                    rowGroups.push_back(dummyGroup);
                }
                dummyGroupTable.push_back(rowGroups);
                count = 0;
            }
        }
    }

    // merge interleaving table and dummy group table
    int dummyRowSize = dummyGroupTable.size();
    int dummyMiddleIndex = dummyRowSize / 2;

    int dummyIndex = 0;
    for (; dummyIndex < dummyMiddleIndex; dummyIndex++)
    {
        interleavingTable.insert(interleavingTable.begin(), dummyGroupTable[dummyIndex]);
    }

    for (; dummyIndex < dummyRowSize; dummyIndex++)
    {
        interleavingTable.push_back(dummyGroupTable[dummyIndex]);
    }


    this->table.clear();
    this->table.resize(rowSize);
    int colNum = interleavingTable.size();
    for (int r = 0; r < rowSize; r++)
    {
        table[r].resize(colNum);
    }

    for (int c = 0; c < colNum; c++)
    {
        for (int r = 0; r < rowSize; r++)
        {
            table[r][c] = interleavingTable[c][r];
        }
    }

    //std::cout << "Interleaving Table:" << std::endl;
    //for (auto& row : this->table)
    //{
    //    for (auto& group : row)
    //    {
    //        std::cout << group.GetSymbolNameSequence() << " ";
    //    }
    //    std::cout << std::endl;
    //}

    return true;
}

void TableManager::CalculateDummyCost()
{
    int nowGroupSize = table[0][0].GetDeviceUnits().size();

    double rowMid = nowGroupSize * static_cast<double>(rowSize - 1) / 2.0;
    double colMid = nowGroupSize * static_cast<double>(colSize - 1) / 2.0;

    double totalCost = 0.0;
    for (int r = 0; r < rowSize; ++r) {
        for (int c = 0; c < colSize; ++c) {
            vector<DeviceUnit> deviceUnits = table[r][c].GetDeviceUnits();
            for (int i = 0; i < deviceUnits.size(); ++i) {
                const DeviceUnit& du = deviceUnits[i];
                if (du.GetSymbol() == "d") {
                    double x = i + c * nowGroupSize;
                    double y = r;
                    // Calculate distance from the center of the table
                    double dist = sqrt((x - colMid) * (x - colMid) + (y - rowMid) * (y - rowMid));
                    totalCost += 1 / dist;
                }
            }

        }
    }
    costMap[CostEnum::dummyCost] = totalCost;
}

// =======================
// Congestion cost (trunk-based, from cCost Part 1)
// =======================
// Uses trunk routing model (SA/SB/DA/DB) to compute per-cell congestion,
// then: per-cell congestion -> distribute to device units -> per-TYPE -> average

void TableManager::CalculateCongestionCost()
{
    using std::string;
    using std::vector;
    using std::unordered_map;
    using std::unordered_set;

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::congestionCost] = 0.0; return; }

    auto is_dummy = [](const string& s) -> bool {
        return s.empty() || s == "d";
        };

    // Trunk positions: SA, SB, DA, DB
    auto trunkRow = [&](int i) -> int {
        if (R % 2 == 0)
            return i * R / 4;
        else
            return (int)std::round(i * (R - 1) / 3.0);
        };
    const int trunkSA = trunkRow(0);
    const int trunkSB = trunkRow(1);
    const int trunkDA = trunkRow(2);
    const int trunkDB = trunkRow(3);

    // Group signature per cell
    auto groupSig = [&](int r, int c) -> string {
        unordered_set<string> s;
        for (const auto& du : table[r][c].GetDeviceUnits())
            if (!is_dummy(du.GetSymbol())) s.insert(du.GetSymbol());
        vector<string> v(s.begin(), s.end());
        std::sort(v.begin(), v.end());
        string sig;
        for (const auto& x : v) sig += x;
        return sig;
        };

    vector<vector<string>> sig(R, vector<string>(C));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            sig[r][c] = groupSig(r, c);

    // Unique groups -> assign trunk pairs
    unordered_set<string> allSigs;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            if (!sig[r][c].empty()) allSigs.insert(sig[r][c]);

    vector<string> groupList(allSigs.begin(), allSigs.end());
    std::sort(groupList.begin(), groupList.end());

    unordered_map<string, int> sigTrunkS, sigTrunkD;
    if (groupList.size() >= 1) { sigTrunkS[groupList[0]] = trunkSA; sigTrunkD[groupList[0]] = trunkDA; }
    if (groupList.size() >= 2) { sigTrunkS[groupList[1]] = trunkSB; sigTrunkD[groupList[1]] = trunkDB; }

    // Count non-dummy units per cell
    vector<vector<int>> cellUnitCount(R, vector<int>(C, 0));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            for (const auto& du : table[r][c].GetDeviceUnits())
                if (!is_dummy(du.GetSymbol())) cellUnitCount[r][c]++;

    // Per-cell congestion: count vertical line passes per cell
    vector<vector<double>> cellCongest(R, vector<double>(C, 0.0));

    for (int c = 0; c < C; ++c)
    {
        struct Line { int rMin; int rMax; };
        vector<Line> lines;

        for (const auto& gSig : groupList)
        {
            if (sigTrunkS.find(gSig) == sigTrunkS.end()) continue;

            vector<int> rows;
            for (int r = 0; r < R; ++r)
                if (sig[r][c] == gSig) rows.push_back(r);
            if (rows.empty()) continue;

            int sTrunk = sigTrunkS[gSig];
            int dTrunk = sigTrunkD[gSig];

            int sMin = sTrunk, sMax = sTrunk;
            for (int r : rows) { sMin = std::min(sMin, r); sMax = std::max(sMax, r); }
            lines.push_back({ sMin, sMax });

            int dMin = dTrunk, dMax = dTrunk;
            for (int r : rows) { dMin = std::min(dMin, r); dMax = std::max(dMax, r); }
            lines.push_back({ dMin, dMax });
        }

        for (int r = 0; r < R; ++r)
        {
            int n = cellUnitCount[r][c];
            if (n == 0) continue;
            int count = 0;
            for (const auto& l : lines)
                if (r >= l.rMin && r <= l.rMax) count++;
            cellCongest[r][c] = static_cast<double>(count) / n;
        }
    }

    // Metric: distribute per-cell congestion to device units, aggregate per TYPE
    unordered_map<string, double> typeSum;
    unordered_map<string, int>    typeCount;

    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c) {
            double m = cellCongest[r][c];
            unordered_map<string, int> cnt;
            for (const auto& du : table[r][c].GetDeviceUnits()) {
                const string& sym = du.GetSymbol();
                if (!is_dummy(sym)) cnt[sym]++;
            }
            for (auto& [sym, n] : cnt) {
                typeSum[sym] += n * m;
                typeCount[sym] += n;
            }
        }

    double sumTypeMetric = 0.0;
    int nTypes = 0;
    for (auto& [sym, total] : typeSum) {
        if (typeCount[sym] > 0) {
            sumTypeMetric += total / typeCount[sym];
            nTypes++;
        }
    }

    costMap[CostEnum::congestionCost] = nTypes > 0 ? sumTypeMetric / nTypes : 0.0;
}

// =======================
// Hierarchical Congestion routing cost
// =======================
// Metric: per-GROUP(=cell) congestion / cellGridCount -> distribute to device units
//         -> per-TYPE sum / device unit count -> average over types

void TableManager::CalculateHierCongestionCost()
{
    // SHARED-tree per-cell distribution + per-type aggregation.
    using std::string;
    using std::vector;

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::hierCongestionCost] = 0.0; return; }

    auto is_dummy = [](const string& s) { return s.empty() || s == "d"; };

    auto groupSig = [&](int r, int c) -> string {
        std::unordered_set<string> s;
        for (const auto& du : table[r][c].GetDeviceUnits())
            if (!is_dummy(du.GetSymbol())) s.insert(du.GetSymbol());
        vector<string> v(s.begin(), s.end());
        std::sort(v.begin(), v.end());
        string sig;
        for (const auto& x : v) sig += x;
        return sig;
        };

    vector<vector<string>> sig(R, vector<string>(C));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            sig[r][c] = groupSig(r, c);

    auto isAllSameSig = [&](int rS, int rE, int cS, int cE) -> bool {
        string single; bool found = false;
        for (int r = rS; r < rE; ++r)
            for (int c = cS; c < cE; ++c) {
                if (sig[r][c].empty()) continue;
                if (!found) { single = sig[r][c]; found = true; }
                else if (sig[r][c] != single) return false;
            }
        return found;
        };
    int cnt_h = 0, cnt_v = 0;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c + 1 < C; ++c)
            if (isAllSameSig(r, r + 1, c, c + 2)) cnt_h++;
    for (int r = 0; r + 1 < R; ++r)
        for (int c = 0; c < C; ++c)
            if (isAllSameSig(r, r + 2, c, c + 1)) cnt_v++;
    const bool h_init = (cnt_h >= cnt_v);

    vector<std::pair<int, int>> steps;
    {
        int rb = 1, cb = 1; bool h = h_init;
        while (rb < R || cb < C) {
            if (h) cb = std::min(cb * 2, C); else rb = std::min(rb * 2, R);
            steps.push_back({ rb, cb });
            if (rb >= R && cb >= C) break;
            h = !h;
        }
        if (steps.empty()) steps.push_back({ R, C });
    }

    auto getPartition = [](int dim) -> vector<int> {
        if (dim <= 0) return {};
        if (dim <= 3) return { dim };
        if (dim % 3 == 0) return vector<int>(dim / 3, 3);
        if (dim % 3 == 2) {
            vector<int> p(1, 2);
            for (int i = 0; i < (dim - 2) / 3; ++i) p.push_back(3);
            return p;
        }
        vector<int> p(1, 2);
        for (int i = 0; i < (dim - 4) / 3; ++i) p.push_back(3);
        p.push_back(2);
        return p;
        };
    auto bucketize = [&](int pos, int dim, int B) -> int {
        if (B >= dim) return 0;
        if (B < 2)    return pos;
        if (dim % 2 == 0) return std::min(pos / B, std::max(dim / B - 1, 0));
        vector<int> part = getPartition(dim);
        int cum = 0;
        for (int i = 0; i < (int)part.size(); ++i) {
            cum += part[i];
            if (pos < cum) return i;
        }
        return (int)part.size() - 1;
        };

    struct Node { double y, x; std::set<string> nets; };
    vector<Node> nodes;
    nodes.reserve((size_t)R * (size_t)C);
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            if (!sig[r][c].empty())
                nodes.push_back({ (double)r, (double)c, { sig[r][c] } });

    struct Edge { int p; int c; };
    vector<Edge> edges;
    vector<int> current;
    current.reserve(nodes.size());
    for (int i = 0; i < (int)nodes.size(); ++i) current.push_back(i);

    for (auto& step : steps) {
        const int rB = step.first; const int cB = step.second;
        std::map<std::pair<int, int>, vector<int>> buckets;
        for (int idx : current) {
            int wr = bucketize((int)nodes[idx].y, R, rB);
            int wc = bucketize((int)nodes[idx].x, C, cB);
            buckets[{wr, wc}].push_back(idx);
        }
        vector<int> new_current;
        new_current.reserve(buckets.size());
        for (auto& kv : buckets) {
            auto& group = kv.second;
            if (group.size() == 1) { new_current.push_back(group[0]); continue; }
            double cy = 0.0, cx = 0.0;
            std::set<string> mn;
            for (int g : group) {
                cy += nodes[g].y; cx += nodes[g].x;
                for (const auto& n : nodes[g].nets) mn.insert(n);
            }
            cy /= (double)group.size();
            cx /= (double)group.size();
            int parentIdx = (int)nodes.size();
            nodes.push_back({ cy, cx, std::move(mn) });
            for (int g : group) edges.push_back({ parentIdx, g });
            new_current.push_back(parentIdx);
        }
        current = std::move(new_current);
    }

    vector<vector<double>> cellMetric(R, vector<double>(C, 0.0));

    auto distributeH = [&](double y, double xa, double xb, double weight) {
        double r_round = std::round(y);
        vector<std::pair<int, double>> rows;
        if (std::abs(y - r_round) < 1e-6) {
            int r = (int)r_round;
            if (r >= 0 && r < R) rows.push_back({ r, 1.0 });
        }
        else {
            int r_low = (int)std::floor(y);
            if (r_low >= 0 && r_low < R) rows.push_back({ r_low,     0.5 });
            if (r_low + 1 >= 0 && r_low + 1 < R) rows.push_back({ r_low + 1, 0.5 });
        }
        double xs = std::min(xa, xb), xe = std::max(xa, xb);
        for (int c = 0; c < C; ++c) {
            double lo = c - 0.5, hi = c + 0.5;
            double ov = std::min(hi, xe) - std::max(lo, xs);
            if (ov > 0.0)
                for (auto& rp : rows) cellMetric[rp.first][c] += ov * rp.second * weight;
        }
        };
    auto distributeV = [&](double x, double ya, double yb, double weight) {
        double c_round = std::round(x);
        vector<std::pair<int, double>> cols;
        if (std::abs(x - c_round) < 1e-6) {
            int c = (int)c_round;
            if (c >= 0 && c < C) cols.push_back({ c, 1.0 });
        }
        else {
            int c_low = (int)std::floor(x);
            if (c_low >= 0 && c_low < C) cols.push_back({ c_low,     0.5 });
            if (c_low + 1 >= 0 && c_low + 1 < C) cols.push_back({ c_low + 1, 0.5 });
        }
        double ys = std::min(ya, yb), ye = std::max(ya, yb);
        for (int r = 0; r < R; ++r) {
            double lo = r - 0.5, hi = r + 0.5;
            double ov = std::min(hi, ye) - std::max(lo, ys);
            if (ov > 0.0)
                for (auto& cp : cols) cellMetric[r][cp.first] += ov * cp.second * weight;
        }
        };

    for (const auto& e : edges) {
        const Node& parent = nodes[e.p];
        const Node& child = nodes[e.c];
        double weight = (double)child.nets.size();
        if (std::abs(parent.x - child.x) > 1e-9)
            distributeH(child.y, child.x, parent.x, weight);
        if (std::abs(parent.y - child.y) > 1e-9)
            distributeV(parent.x, child.y, parent.y, weight);
    }

    std::unordered_map<string, double> typeSum;
    std::unordered_map<string, int>    typeCount;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c) {
            double m = cellMetric[r][c];
            std::unordered_map<string, int> cnt;
            for (const auto& du : table[r][c].GetDeviceUnits()) {
                const string& sym = du.GetSymbol();
                if (!is_dummy(sym)) cnt[sym]++;
            }
            for (auto& kv : cnt) {
                typeSum[kv.first] += kv.second * m;
                typeCount[kv.first] += kv.second;
            }
        }

    double sumTypeMetric = 0.0;
    int nTypes = 0;
    for (auto& kv : typeSum) {
        if (typeCount[kv.first] > 0) {
            sumTypeMetric += kv.second / typeCount[kv.first];
            nTypes++;
        }
    }
    costMap[CostEnum::hierCongestionCost] = nTypes > 0 ? sumTypeMetric / nTypes : 0.0;
}

#if 0
void TableManager::CalculateHierCongestionCost_OLD_DeviceUnitLevel()
{
    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::hierCongestionCost] = 0.0; return; }

    // --- helpers ---
    auto is_dummy = [](const std::string& s) { return s.empty() || s == "d"; };
    auto normPairFn = [](char a, char b) -> std::string { return { std::min(a,b), std::max(a,b) }; };

    static const int NUM_SUB = 4;

    // Build pattern strings per cell (e.g. "AAACCCCA")
    std::vector<std::vector<std::string>> cellPat(R, std::vector<std::string>(C));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            std::string s;
            for (const auto& du : table[r][c].GetDeviceUnits())
                s += du.GetSymbol();
            cellPat[r][c] = s;
        }

    int patLen = (int)cellPat[0][0].size();
    if (patLen < 2) { costMap[CostEnum::hierCongestionCost] = 0.0; return; }
    int gapsPerCell = patLen - 1;

    auto netAtPos = [&](int r, int c, int gi) -> std::string {
        return normPairFn(cellPat[r][c][gi], cellPat[r][c][gi + 1]);
        };

    // Auto-detect target nets, exclude outer letter same-pairs
    std::set<char> outerLetters;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            outerLetters.insert(cellPat[r][c][0]);

    std::set<std::string> targetNets;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int gi = 0; gi < gapsPerCell; gi++) {
                std::string net = netAtPos(r, c, gi);
                if (net[0] == net[1] && outerLetters.count(net[0])) continue;
                targetNets.insert(net);
            }

    int gridRows = R * NUM_SUB;
    int gridCols = C * gapsPerCell;

    // Per-cell sub-row assignment (order of first appearance)
    std::vector<std::vector<std::map<std::string, int>>> cellSubRow(R, std::vector<std::map<std::string, int>>(C));
    std::vector<std::vector<std::map<std::string, int>>> cellRepGap(R, std::vector<std::map<std::string, int>>(C));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            int nextSub = 0;
            std::set<std::string> seen;
            for (int gi = 0; gi < gapsPerCell; gi++) {
                std::string net = netAtPos(r, c, gi);
                if (!targetNets.count(net)) continue;
                if (seen.insert(net).second) {
                    cellSubRow[r][c][net] = nextSub++;
                    cellRepGap[r][c][net] = gi;
                }
            }
        }

    // --- UnionFind ---
    struct UF {
        std::vector<int> p;
        UF() {}
        UF(int n) : p(n) { for (int i = 0; i < n; i++) p[i] = i; }
        int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
        void unite(int a, int b) { a = find(a); b = find(b); if (a != b) p[a] = b; }
        bool connected(int a, int b) { return find(a) == find(b); }
    };

    auto cid = [&](int r, int c) { return r * gridCols + c; };
    auto toGC = [&](int cell, int gi) { return cell * gapsPerCell + gi; };

    auto isSeed = [&](int sr, int gc) -> bool {
        int origRow = sr / NUM_SUB;
        int cellCol = gc / gapsPerCell;
        int gi = gc % gapsPerCell;
        int subIdx = sr % NUM_SUB;
        std::string net = netAtPos(origRow, cellCol, gi);
        if (!targetNets.count(net)) return false;
        auto it = cellSubRow[origRow][cellCol].find(net);
        if (it == cellSubRow[origRow][cellCol].end() || it->second != subIdx) return false;
        auto it2 = cellRepGap[origRow][cellCol].find(net);
        return it2 != cellRepGap[origRow][cellCol].end() && it2->second == gi;
        };

    int totalCells = gridRows * gridCols;
    std::map<std::string, UF> nameUF;
    for (auto& n : targetNets)
        nameUF[n] = UF(totalCells);

    // Congestion accumulator
    std::vector<std::vector<int>> congestion(gridRows, std::vector<int>(gridCols, 0));

    auto addLayer = [&](const std::vector<std::vector<int>>& L) {
        for (int r = 0; r < gridRows; r++)
            for (int c = 0; c < gridCols; c++)
                congestion[r][c] += L[r][c];
        };

    // Connect helpers
    auto connectH = [&](int sr, int gc1, int gc2, std::vector<std::vector<int>>& L, const std::string& name) {
        auto& uf = nameUF[name];
        int gMin = std::min(gc1, gc2), gMax = std::max(gc1, gc2);
        for (int g = gMin; g <= gMax; g++) {
            if (L[sr][g] == 0) L[sr][g] = 1;
            uf.unite(cid(sr, gc1), cid(sr, g));
        }
        };

    auto connectV = [&](int gc, int sr1, int sr2, std::vector<std::vector<int>>& L, const std::string& name) {
        auto& uf = nameUF[name];
        int sMin = std::min(sr1, sr2), sMax = std::max(sr1, sr2);
        for (int sr = sMin; sr <= sMax; sr++) {
            if (L[sr][gc] == 0) L[sr][gc] = 1;
            uf.unite(cid(sr1, gc), cid(sr, gc));
        }
        };

    auto connectLShape = [&](int sr1, int gc1, int sr2, int gc2,
        std::vector<std::vector<int>>& Lv, std::vector<std::vector<int>>& Lh,
        const std::string& name) {
            auto& uf = nameUF[name];
            int sMin = std::min(sr1, sr2), sMax = std::max(sr1, sr2);
            for (int sr = sMin; sr <= sMax; sr++) {
                if (Lv[sr][gc1] == 0) Lv[sr][gc1] = 1;
                uf.unite(cid(sr1, gc1), cid(sr, gc1));
            }
            int gMin = std::min(gc1, gc2), gMax = std::max(gc1, gc2);
            for (int g = gMin; g <= gMax; g++) {
                if (Lh[sr2][g] == 0) Lh[sr2][g] = 1;
                uf.unite(cid(sr1, gc1), cid(sr2, g));
            }
        };

    // ============================================================
    // LG: vertical conductors within each cell (UF only, no congestion)
    // ============================================================
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int gi = 0; gi < gapsPerCell; gi++) {
                std::string net = netAtPos(r, c, gi);
                if (!targetNets.count(net)) continue;
                int gc = toGC(c, gi);
                int srBase = r * NUM_SUB;
                auto& uf = nameUF[net];
                for (int s = 1; s < NUM_SUB; s++)
                    uf.unite(cid(srBase, gc), cid(srBase + s, gc));
            }

    // ============================================================
    // LG1: intra-cell horizontal same-name connection
    // ============================================================
    {
        std::vector<std::vector<int>> L(gridRows, std::vector<int>(gridCols, 0));
        for (int r = 0; r < R; r++)
            for (int c = 0; c < C; c++) {
                std::map<std::string, std::vector<int>> netGaps;
                for (int gi = 0; gi < gapsPerCell; gi++) {
                    std::string net = netAtPos(r, c, gi);
                    if (targetNets.count(net))
                        netGaps[net].push_back(gi);
                }
                for (auto& [net, gis] : netGaps) {
                    if (gis.size() < 2) continue;
                    auto it = cellSubRow[r][c].find(net);
                    if (it == cellSubRow[r][c].end()) continue;
                    int subIdx = it->second;
                    int sr = r * NUM_SUB + subIdx;
                    int gcFirst = toGC(c, gis.front());
                    int gcLast = toGC(c, gis.back());
                    auto& uf = nameUF[net];
                    for (int g = gcFirst; g <= gcLast; g++) {
                        if (L[sr][g] == 0) L[sr][g] = 1;
                        uf.unite(cid(sr, gcFirst), cid(sr, g));
                    }
                }
            }
        addLayer(L);
    }

    // ============================================================
    // L0: cross-cell vertical for 2-row pairs (same gi & same sub-row only)
    // ============================================================
    {
        std::vector<std::vector<int>> L(gridRows, std::vector<int>(gridCols, 0));
        for (int rPair = 0; rPair + 1 < R; rPair += 2)
            for (int c = 0; c < C; c++)
                for (auto& net : targetNets) {
                    auto& rep0 = cellRepGap[rPair][c];
                    auto& rep1 = cellRepGap[rPair + 1][c];
                    auto& sub0 = cellSubRow[rPair][c];
                    auto& sub1 = cellSubRow[rPair + 1][c];
                    bool has0 = rep0.count(net) > 0;
                    bool has1 = rep1.count(net) > 0;
                    if (!has0 || !has1) continue;
                    if (rep0.at(net) != rep1.at(net) || sub0.at(net) != sub1.at(net)) continue;
                    int gi = rep0.at(net);
                    int gc = toGC(c, gi);
                    int subIdx = sub0.at(net);
                    int sr0 = rPair * NUM_SUB + subIdx;
                    int sr1 = (rPair + 1) * NUM_SUB + subIdx;
                    auto& uf = nameUF[net];
                    for (int sr = sr0; sr <= sr1; sr++) {
                        L[sr][gc] = 1;
                        uf.unite(cid(sr0, gc), cid(sr, gc));
                    }
                }
        addLayer(L);
    }

    // ============================================================
    // Expansion layers: H/V merge with increasing block sizes
    // ============================================================
    {
        struct Step { int rBlk, cBlk; bool horiz; };
        std::vector<Step> steps;
        int rb = 2, cb = 1;
        bool h = true;
        while (rb < R || cb < C) {
            if (h) cb = std::min(cb * 2, C);
            else   rb = std::min(rb * 2, R);
            steps.push_back({ rb, cb, h });
            if (rb >= R && cb >= C) break;
            h = !h;
        }
        if (steps.empty()) steps.push_back({ R, C, true });

        for (auto& [rBlk, cBlk, horiz] : steps) {
            std::vector<std::vector<int>> L(gridRows, std::vector<int>(gridCols, 0));
            bool anyChange = false;

            for (int rS = 0; rS < R; rS += rBlk) {
                int rE = std::min(rS + rBlk, R);
                for (int cS = 0; cS < C; cS += cBlk) {
                    int cE = std::min(cS + cBlk, C);
                    int srS = rS * NUM_SUB;
                    int srE = rE * NUM_SUB;
                    int gcS = cS * gapsPerCell;
                    int gcE = cE * gapsPerCell;

                    for (const std::string& name : targetNets) {
                        std::vector<std::pair<int, int>> seeds;
                        for (int sr = srS; sr < srE; sr++)
                            for (int gc = gcS; gc < gcE; gc++)
                                if (isSeed(sr, gc) &&
                                    netAtPos(sr / NUM_SUB, gc / gapsPerCell, gc % gapsPerCell) == name)
                                    seeds.push_back({ sr, gc });
                        if (seeds.size() < 2) continue;

                        auto& uf = nameUF[name];
                        if (horiz) {
                            std::map<int, std::vector<int>> srToGCs;
                            for (auto& [sr, gc] : seeds) srToGCs[sr].push_back(gc);
                            std::vector<std::pair<int, std::vector<int>>> sorted_vec(srToGCs.begin(), srToGCs.end());
                            for (auto& [sr, gcs] : sorted_vec) std::sort(gcs.begin(), gcs.end());
                            std::sort(sorted_vec.begin(), sorted_vec.end(), [](auto& a, auto& b) {
                                return (a.second.back() - a.second.front()) >
                                    (b.second.back() - b.second.front());
                                });
                            for (auto& [sr, gcs] : sorted_vec) {
                                for (int i = 1; i < (int)gcs.size(); i++) {
                                    if (!uf.connected(cid(sr, gcs[i - 1]), cid(sr, gcs[i]))) {
                                        connectH(sr, gcs[i - 1], gcs[i], L, name);
                                        anyChange = true;
                                    }
                                }
                            }
                        }
                        else {
                            std::map<int, std::vector<int>> gcToSRs;
                            for (auto& [sr, gc] : seeds) gcToSRs[gc].push_back(sr);
                            std::vector<std::pair<int, std::vector<int>>> sorted_vec(gcToSRs.begin(), gcToSRs.end());
                            for (auto& [gc, srs] : sorted_vec) std::sort(srs.begin(), srs.end());
                            std::sort(sorted_vec.begin(), sorted_vec.end(), [](auto& a, auto& b) {
                                return (a.second.back() - a.second.front()) >
                                    (b.second.back() - b.second.front());
                                });
                            for (auto& [gc, srs] : sorted_vec) {
                                for (int i = 1; i < (int)srs.size(); i++) {
                                    if (!uf.connected(cid(srs[i - 1], gc), cid(srs[i], gc))) {
                                        connectV(gc, srs[i - 1], srs[i], L, name);
                                        anyChange = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            if (anyChange) addLayer(L);
        }
    }

    // ============================================================
    // L-shape for remaining unconnected seeds
    // ============================================================
    {
        std::vector<std::vector<int>> Lv(gridRows, std::vector<int>(gridCols, 0));
        std::vector<std::vector<int>> Lh(gridRows, std::vector<int>(gridCols, 0));

        for (const std::string& name : targetNets) {
            std::vector<std::pair<int, int>> seeds;
            for (int sr = 0; sr < gridRows; sr++)
                for (int gc = 0; gc < gridCols; gc++)
                    if (isSeed(sr, gc) &&
                        netAtPos(sr / NUM_SUB, gc / gapsPerCell, gc % gapsPerCell) == name)
                        seeds.push_back({ sr, gc });
            if (seeds.size() < 2) continue;

            auto& uf = nameUF[name];
            for (int i = 1; i < (int)seeds.size(); i++) {
                auto [s0, g0] = seeds[0];
                auto [si, gi] = seeds[i];
                if (!uf.connected(cid(s0, g0), cid(si, gi)))
                    connectLShape(s0, g0, si, gi, Lv, Lh, name);
            }
        }

        bool hasV = false, hasH = false;
        for (int r = 0; r < gridRows && !hasV; r++)
            for (int c = 0; c < gridCols && !hasV; c++)
                if (Lv[r][c] > 0) hasV = true;
        for (int r = 0; r < gridRows && !hasH; r++)
            for (int c = 0; c < gridCols && !hasH; c++)
                if (Lh[r][c] > 0) hasH = true;

        if (hasV) addLayer(Lv);
        if (hasH) addLayer(Lh);
    }

    // ================================================= ===========
    // Metric calculation
    // ============================================================

    // Step 1-2: per-cell congestion sum
    std::vector<std::vector<int>> cellCong(R, std::vector<int>(C, 0));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int sr = 0; sr < NUM_SUB; sr++) {
                int gr = r * NUM_SUB + sr;
                for (int gi = 0; gi < gapsPerCell; gi++) {
                    int gc = c * gapsPerCell + gi;
                    cellCong[r][c] += congestion[gr][gc];
                }
            }

    // Step 3: GROUP metric = cellCong / cellGridCount (NUM_SUB * gapsPerCell)
    int cellGridCount = NUM_SUB * gapsPerCell;
    std::vector<std::vector<double>> cellMetric(R, std::vector<double>(C, 0.0));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            cellMetric[r][c] = (double)cellCong[r][c] / cellGridCount;

    // Step 4-5: distribute GROUP metric to each device unit, aggregate per TYPE
    std::unordered_map<std::string, double> typeSum;   // type -> sum(count * metric)
    std::unordered_map<std::string, int>    typeCount; // type -> total device units

    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            double m = cellMetric[r][c];
            std::unordered_map<std::string, int> cnt;
            for (const auto& du : table[r][c].GetDeviceUnits()) {
                const std::string& sym = du.GetSymbol();
                if (!is_dummy(sym)) cnt[sym]++;
            }
            for (auto& [sym, n] : cnt) {
                typeSum[sym] += n * m;
                typeCount[sym] += n;
            }
        }

    // Step 6: final = average of per-TYPE metrics
    double sumTypeMetric = 0.0;
    int nTypes = 0;
    for (auto& [sym, total] : typeSum) {
        if (typeCount[sym] > 0) {
            sumTypeMetric += total / typeCount[sym];
            nTypes++;
        }
    }

    costMap[CostEnum::hierCongestionCost] = nTypes > 0 ? sumTypeMetric / nTypes : 0.0;
}
#endif // disabled old device-unit-level HierCongestion

// =======================
// Hierarchical cCost (cCost with hierarchical congestion replacing trunk-based)
// =======================
// Part 1: hierarchical routing congestion (same as CalculateHierCongestionCost)
// Part 2: lateral H/V coupling (same as original cCost Part 2)

void TableManager::CalculateHierCCost()
{
    using std::string;
    using std::vector;
    using std::unordered_map;

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) { costMap[CostEnum::hierCCost] = 0.0; return; }

    auto is_dummy = [](const string& s) { return s.empty() || s == "d"; };

    // ============================================================
    // [Part 1] SHARED-tree per-cell distribution -> cellCongest
    // ============================================================
    auto groupSig = [&](int r, int c) -> string {
        std::unordered_set<string> s;
        for (const auto& du : table[r][c].GetDeviceUnits())
            if (!is_dummy(du.GetSymbol())) s.insert(du.GetSymbol());
        vector<string> v(s.begin(), s.end());
        std::sort(v.begin(), v.end());
        string sig;
        for (const auto& x : v) sig += x;
        return sig;
        };

    vector<vector<string>> sig(R, vector<string>(C));
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            sig[r][c] = groupSig(r, c);

    auto isAllSameSig = [&](int rS, int rE, int cS, int cE) -> bool {
        string single; bool found = false;
        for (int r = rS; r < rE; ++r)
            for (int c = cS; c < cE; ++c) {
                if (sig[r][c].empty()) continue;
                if (!found) { single = sig[r][c]; found = true; }
                else if (sig[r][c] != single) return false;
            }
        return found;
        };
    int cnt_h = 0, cnt_v = 0;
    for (int r = 0; r < R; ++r)
        for (int c = 0; c + 1 < C; ++c)
            if (isAllSameSig(r, r + 1, c, c + 2)) cnt_h++;
    for (int r = 0; r + 1 < R; ++r)
        for (int c = 0; c < C; ++c)
            if (isAllSameSig(r, r + 2, c, c + 1)) cnt_v++;
    const bool h_init = (cnt_h >= cnt_v);

    vector<std::pair<int, int>> steps;
    {
        int rb = 1, cb = 1; bool h = h_init;
        while (rb < R || cb < C) {
            if (h) cb = std::min(cb * 2, C); else rb = std::min(rb * 2, R);
            steps.push_back({ rb, cb });
            if (rb >= R && cb >= C) break;
            h = !h;
        }
        if (steps.empty()) steps.push_back({ R, C });
    }

    auto getPartition = [](int dim) -> vector<int> {
        if (dim <= 0) return {};
        if (dim <= 3) return { dim };
        if (dim % 3 == 0) return vector<int>(dim / 3, 3);
        if (dim % 3 == 2) {
            vector<int> p(1, 2);
            for (int i = 0; i < (dim - 2) / 3; ++i) p.push_back(3);
            return p;
        }
        vector<int> p(1, 2);
        for (int i = 0; i < (dim - 4) / 3; ++i) p.push_back(3);
        p.push_back(2);
        return p;
        };
    auto bucketize = [&](int pos, int dim, int B) -> int {
        if (B >= dim) return 0;
        if (B < 2)    return pos;
        if (dim % 2 == 0) return std::min(pos / B, std::max(dim / B - 1, 0));
        vector<int> part = getPartition(dim);
        int cum = 0;
        for (int i = 0; i < (int)part.size(); ++i) {
            cum += part[i];
            if (pos < cum) return i;
        }
        return (int)part.size() - 1;
        };

    struct Node { double y, x; std::set<string> nets; };
    vector<Node> nodes;
    nodes.reserve((size_t)R * (size_t)C);
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            if (!sig[r][c].empty())
                nodes.push_back({ (double)r, (double)c, { sig[r][c] } });

    struct Edge { int p; int c; };
    vector<Edge> edges;
    vector<int> current;
    current.reserve(nodes.size());
    for (int i = 0; i < (int)nodes.size(); ++i) current.push_back(i);

    for (auto& step : steps) {
        const int rB = step.first; const int cB = step.second;
        std::map<std::pair<int, int>, vector<int>> buckets;
        for (int idx : current) {
            int wr = bucketize((int)nodes[idx].y, R, rB);
            int wc = bucketize((int)nodes[idx].x, C, cB);
            buckets[{wr, wc}].push_back(idx);
        }
        vector<int> new_current;
        new_current.reserve(buckets.size());
        for (auto& kv : buckets) {
            auto& group = kv.second;
            if (group.size() == 1) { new_current.push_back(group[0]); continue; }
            double cy = 0.0, cx = 0.0;
            std::set<string> mn;
            for (int g : group) {
                cy += nodes[g].y; cx += nodes[g].x;
                for (const auto& n : nodes[g].nets) mn.insert(n);
            }
            cy /= (double)group.size();
            cx /= (double)group.size();
            int parentIdx = (int)nodes.size();
            nodes.push_back({ cy, cx, std::move(mn) });
            for (int g : group) edges.push_back({ parentIdx, g });
            new_current.push_back(parentIdx);
        }
        current = std::move(new_current);
    }

    vector<vector<double>> cellCongest(R, vector<double>(C, 0.0));

    auto distributeH = [&](double y, double xa, double xb, double weight) {
        double r_round = std::round(y);
        vector<std::pair<int, double>> rows;
        if (std::abs(y - r_round) < 1e-6) {
            int r = (int)r_round;
            if (r >= 0 && r < R) rows.push_back({ r, 1.0 });
        }
        else {
            int r_low = (int)std::floor(y);
            if (r_low >= 0 && r_low < R) rows.push_back({ r_low,     0.5 });
            if (r_low + 1 >= 0 && r_low + 1 < R) rows.push_back({ r_low + 1, 0.5 });
        }
        double xs = std::min(xa, xb), xe = std::max(xa, xb);
        for (int c = 0; c < C; ++c) {
            double lo = c - 0.5, hi = c + 0.5;
            double ov = std::min(hi, xe) - std::max(lo, xs);
            if (ov > 0.0)
                for (auto& rp : rows) cellCongest[rp.first][c] += ov * rp.second * weight;
        }
        };
    auto distributeV = [&](double x, double ya, double yb, double weight) {
        double c_round = std::round(x);
        vector<std::pair<int, double>> cols;
        if (std::abs(x - c_round) < 1e-6) {
            int c = (int)c_round;
            if (c >= 0 && c < C) cols.push_back({ c, 1.0 });
        }
        else {
            int c_low = (int)std::floor(x);
            if (c_low >= 0 && c_low < C) cols.push_back({ c_low,     0.5 });
            if (c_low + 1 >= 0 && c_low + 1 < C) cols.push_back({ c_low + 1, 0.5 });
        }
        double ys = std::min(ya, yb), ye = std::max(ya, yb);
        for (int r = 0; r < R; ++r) {
            double lo = r - 0.5, hi = r + 0.5;
            double ov = std::min(hi, ye) - std::max(lo, ys);
            if (ov > 0.0)
                for (auto& cp : cols) cellCongest[r][cp.first] += ov * cp.second * weight;
        }
        };

    for (const auto& e : edges) {
        const Node& parent = nodes[e.p];
        const Node& child = nodes[e.c];
        double weight = (double)child.nets.size();
        if (std::abs(parent.x - child.x) > 1e-9)
            distributeH(child.y, child.x, parent.x, weight);
        if (std::abs(parent.y - child.y) > 1e-9)
            distributeV(parent.x, child.y, parent.y, weight);
    }

#if 0
    auto normPairFn = [](char a, char b) -> string { return { std::min(a,b), std::max(a,b) }; };

    static const int NUM_SUB = 4;

    vector<vector<string>> cellPat(R, vector<string>(C));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            string s;
            for (const auto& du : table[r][c].GetDeviceUnits())
                s += du.GetSymbol();
            cellPat[r][c] = s;
        }

    int patLen = (int)cellPat[0][0].size();
    if (patLen < 2) { costMap[CostEnum::hierCCost] = 0.0; return; }
    int gapsPerCell = patLen - 1;

    auto netAtPos = [&](int r, int c, int gi) -> string {
        return normPairFn(cellPat[r][c][gi], cellPat[r][c][gi + 1]);
        };

    // Auto-detect target nets, exclude outer letter same-pairs
    std::set<char> outerLetters;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            outerLetters.insert(cellPat[r][c][0]);

    std::set<string> targetNets;
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int gi = 0; gi < gapsPerCell; gi++) {
                string net = netAtPos(r, c, gi);
                if (net[0] == net[1] && outerLetters.count(net[0])) continue;
                targetNets.insert(net);
            }

    int gridRows = R * NUM_SUB;
    int gridCols = C * gapsPerCell;

    // Per-cell sub-row assignment (order of first appearance)
    vector<vector<std::map<string, int>>> cellSubRow(R, vector<std::map<string, int>>(C));
    vector<vector<std::map<string, int>>> cellRepGap(R, vector<std::map<string, int>>(C));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            int nextSub = 0;
            std::set<string> seen;
            for (int gi = 0; gi < gapsPerCell; gi++) {
                string net = netAtPos(r, c, gi);
                if (!targetNets.count(net)) continue;
                if (seen.insert(net).second) {
                    cellSubRow[r][c][net] = nextSub++;
                    cellRepGap[r][c][net] = gi;
                }
            }
        }

    // UnionFind
    struct UF {
        vector<int> p;
        UF() {}
        UF(int n) : p(n) { for (int i = 0; i < n; i++) p[i] = i; }
        int find(int x) { while (p[x] != x) { p[x] = p[p[x]]; x = p[x]; } return x; }
        void unite(int a, int b) { a = find(a); b = find(b); if (a != b) p[a] = b; }
        bool connected(int a, int b) { return find(a) == find(b); }
    };

    auto cid = [&](int r, int c) { return r * gridCols + c; };
    auto toGC = [&](int cell, int gi) { return cell * gapsPerCell + gi; };

    auto isSeed = [&](int sr, int gc) -> bool {
        int origRow = sr / NUM_SUB;
        int cellCol = gc / gapsPerCell;
        int gi = gc % gapsPerCell;
        int subIdx = sr % NUM_SUB;
        string net = netAtPos(origRow, cellCol, gi);
        if (!targetNets.count(net)) return false;
        auto it = cellSubRow[origRow][cellCol].find(net);
        if (it == cellSubRow[origRow][cellCol].end() || it->second != subIdx) return false;
        auto it2 = cellRepGap[origRow][cellCol].find(net);
        return it2 != cellRepGap[origRow][cellCol].end() && it2->second == gi;
        };

    int totalCells = gridRows * gridCols;
    std::map<string, UF> nameUF;
    for (auto& n : targetNets)
        nameUF[n] = UF(totalCells);

    vector<vector<int>> congestion(gridRows, vector<int>(gridCols, 0));

    auto addLayer = [&](const vector<vector<int>>& L) {
        for (int r = 0; r < gridRows; r++)
            for (int c = 0; c < gridCols; c++)
                congestion[r][c] += L[r][c];
        };

    auto connectH = [&](int sr, int gc1, int gc2, vector<vector<int>>& L, const string& name) {
        auto& uf = nameUF[name];
        int gMin = std::min(gc1, gc2), gMax = std::max(gc1, gc2);
        for (int g = gMin; g <= gMax; g++) {
            if (L[sr][g] == 0) L[sr][g] = 1;
            uf.unite(cid(sr, gc1), cid(sr, g));
        }
        };

    auto connectV = [&](int gc, int sr1, int sr2, vector<vector<int>>& L, const string& name) {
        auto& uf = nameUF[name];
        int sMin = std::min(sr1, sr2), sMax = std::max(sr1, sr2);
        for (int sr = sMin; sr <= sMax; sr++) {
            if (L[sr][gc] == 0) L[sr][gc] = 1;
            uf.unite(cid(sr1, gc), cid(sr, gc));
        }
        };

    auto connectLShape = [&](int sr1, int gc1, int sr2, int gc2,
        vector<vector<int>>& Lv, vector<vector<int>>& Lh,
        const string& name) {
            auto& uf = nameUF[name];
            int sMin = std::min(sr1, sr2), sMax = std::max(sr1, sr2);
            for (int sr = sMin; sr <= sMax; sr++) {
                if (Lv[sr][gc1] == 0) Lv[sr][gc1] = 1;
                uf.unite(cid(sr1, gc1), cid(sr, gc1));
            }
            int gMin = std::min(gc1, gc2), gMax = std::max(gc1, gc2);
            for (int g = gMin; g <= gMax; g++) {
                if (Lh[sr2][g] == 0) Lh[sr2][g] = 1;
                uf.unite(cid(sr1, gc1), cid(sr2, g));
            }
        };

    // LG: vertical conductors within each cell (UF only, no congestion)
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++)
            for (int gi = 0; gi < gapsPerCell; gi++) {
                string net = netAtPos(r, c, gi);
                if (!targetNets.count(net)) continue;
                int gc = toGC(c, gi);
                int srBase = r * NUM_SUB;
                auto& uf = nameUF[net];
                for (int s = 1; s < NUM_SUB; s++)
                    uf.unite(cid(srBase, gc), cid(srBase + s, gc));
            }

    // LG1: intra-cell horizontal same-name connection
    {
        vector<vector<int>> L(gridRows, vector<int>(gridCols, 0));
        for (int r = 0; r < R; r++)
            for (int c = 0; c < C; c++) {
                std::map<string, vector<int>> netGaps;
                for (int gi = 0; gi < gapsPerCell; gi++) {
                    string net = netAtPos(r, c, gi);
                    if (targetNets.count(net))
                        netGaps[net].push_back(gi);
                }
                for (auto& [net, gis] : netGaps) {
                    if (gis.size() < 2) continue;
                    auto it = cellSubRow[r][c].find(net);
                    if (it == cellSubRow[r][c].end()) continue;
                    int subIdx = it->second;
                    int sr = r * NUM_SUB + subIdx;
                    int gcFirst = toGC(c, gis.front());
                    int gcLast = toGC(c, gis.back());
                    auto& uf = nameUF[net];
                    for (int g = gcFirst; g <= gcLast; g++) {
                        if (L[sr][g] == 0) L[sr][g] = 1;
                        uf.unite(cid(sr, gcFirst), cid(sr, g));
                    }
                }
            }
        addLayer(L);
    }

    // L0: cross-cell vertical for 2-row pairs (same gi & same sub-row only)
    {
        vector<vector<int>> L(gridRows, vector<int>(gridCols, 0));
        for (int rPair = 0; rPair + 1 < R; rPair += 2)
            for (int c = 0; c < C; c++)
                for (auto& net : targetNets) {
                    auto& rep0 = cellRepGap[rPair][c];
                    auto& rep1 = cellRepGap[rPair + 1][c];
                    auto& sub0 = cellSubRow[rPair][c];
                    auto& sub1 = cellSubRow[rPair + 1][c];
                    bool has0 = rep0.count(net) > 0;
                    bool has1 = rep1.count(net) > 0;
                    if (!has0 || !has1) continue;
                    if (rep0.at(net) != rep1.at(net) || sub0.at(net) != sub1.at(net)) continue;
                    int gi = rep0.at(net);
                    int gc = toGC(c, gi);
                    int subIdx = sub0.at(net);
                    int sr0 = rPair * NUM_SUB + subIdx;
                    int sr1 = (rPair + 1) * NUM_SUB + subIdx;
                    auto& uf = nameUF[net];
                    for (int sr = sr0; sr <= sr1; sr++) {
                        L[sr][gc] = 1;
                        uf.unite(cid(sr0, gc), cid(sr, gc));
                    }
                }
        addLayer(L);
    }

    // Expansion layers: H/V merge with increasing block sizes
    {
        struct Step { int rBlk, cBlk; bool horiz; };
        vector<Step> steps;
        int rb = 2, cb = 1;
        bool h = true;
        while (rb < R || cb < C) {
            if (h) cb = std::min(cb * 2, C);
            else   rb = std::min(rb * 2, R);
            steps.push_back({ rb, cb, h });
            if (rb >= R && cb >= C) break;
            h = !h;
        }
        if (steps.empty()) steps.push_back({ R, C, true });

        for (auto& [rBlk, cBlk, horiz] : steps) {
            vector<vector<int>> L(gridRows, vector<int>(gridCols, 0));
            bool anyChange = false;
            for (int rS = 0; rS < R; rS += rBlk) {
                int rE = std::min(rS + rBlk, R);
                for (int cS = 0; cS < C; cS += cBlk) {
                    int cE = std::min(cS + cBlk, C);
                    int srS = rS * NUM_SUB, srE = rE * NUM_SUB;
                    int gcS = cS * gapsPerCell, gcE = cE * gapsPerCell;
                    for (const string& name : targetNets) {
                        vector<std::pair<int, int>> seeds;
                        for (int sr = srS; sr < srE; sr++)
                            for (int gc = gcS; gc < gcE; gc++)
                                if (isSeed(sr, gc) &&
                                    netAtPos(sr / NUM_SUB, gc / gapsPerCell, gc % gapsPerCell) == name)
                                    seeds.push_back({ sr, gc });
                        if (seeds.size() < 2) continue;
                        auto& uf = nameUF[name];
                        if (horiz) {
                            std::map<int, vector<int>> srToGCs;
                            for (auto& [sr, gc] : seeds) srToGCs[sr].push_back(gc);
                            vector<std::pair<int, vector<int>>> sorted_vec(srToGCs.begin(), srToGCs.end());
                            for (auto& [sr, gcs] : sorted_vec) std::sort(gcs.begin(), gcs.end());
                            std::sort(sorted_vec.begin(), sorted_vec.end(), [](auto& a, auto& b) {
                                return (a.second.back() - a.second.front()) >
                                    (b.second.back() - b.second.front());
                                });
                            for (auto& [sr, gcs] : sorted_vec) {
                                for (int i = 1; i < (int)gcs.size(); i++)
                                    if (!uf.connected(cid(sr, gcs[i - 1]), cid(sr, gcs[i]))) {
                                        connectH(sr, gcs[i - 1], gcs[i], L, name);
                                        anyChange = true;
                                    }
                            }
                        }
                        else {
                            std::map<int, vector<int>> gcToSRs;
                            for (auto& [sr, gc] : seeds) gcToSRs[gc].push_back(sr);
                            vector<std::pair<int, vector<int>>> sorted_vec(gcToSRs.begin(), gcToSRs.end());
                            for (auto& [gc, srs] : sorted_vec) std::sort(srs.begin(), srs.end());
                            std::sort(sorted_vec.begin(), sorted_vec.end(), [](auto& a, auto& b) {
                                return (a.second.back() - a.second.front()) >
                                    (b.second.back() - b.second.front());
                                });
                            for (auto& [gc, srs] : sorted_vec) {
                                for (int i = 1; i < (int)srs.size(); i++)
                                    if (!uf.connected(cid(srs[i - 1], gc), cid(srs[i], gc))) {
                                        connectV(gc, srs[i - 1], srs[i], L, name);
                                        anyChange = true;
                                    }
                            }
                        }
                    }
                }
            }
            if (anyChange) addLayer(L);
        }
    }

    // L-shape for remaining unconnected seeds
    {
        vector<vector<int>> Lv(gridRows, vector<int>(gridCols, 0));
        vector<vector<int>> Lh(gridRows, vector<int>(gridCols, 0));
        for (const string& name : targetNets) {
            vector<std::pair<int, int>> seeds;
            for (int sr = 0; sr < gridRows; sr++)
                for (int gc = 0; gc < gridCols; gc++)
                    if (isSeed(sr, gc) &&
                        netAtPos(sr / NUM_SUB, gc / gapsPerCell, gc % gapsPerCell) == name)
                        seeds.push_back({ sr, gc });
            if (seeds.size() < 2) continue;
            auto& uf = nameUF[name];
            for (int i = 1; i < (int)seeds.size(); i++) {
                auto [s0, g0] = seeds[0];
                auto [si, gi] = seeds[i];
                if (!uf.connected(cid(s0, g0), cid(si, gi)))
                    connectLShape(s0, g0, si, gi, Lv, Lh, name);
            }
        }
        bool hasV = false, hasH = false;
        for (int r = 0; r < gridRows && !hasV; r++)
            for (int c = 0; c < gridCols && !hasV; c++)
                if (Lv[r][c] > 0) hasV = true;
        for (int r = 0; r < gridRows && !hasH; r++)
            for (int c = 0; c < gridCols && !hasH; c++)
                if (Lh[r][c] > 0) hasH = true;
        if (hasV) addLayer(Lv);
        if (hasH) addLayer(Lh);
    }

    // Old per-cell aggregation (disabled; cellCongest produced by SHARED-tree above)
    int cellGridCount = NUM_SUB * gapsPerCell;
    vector<vector<double>> cellCongest_OLD(R, vector<double>(C, 0.0));
    for (int r = 0; r < R; r++)
        for (int c = 0; c < C; c++) {
            int s = 0;
            for (int sr = 0; sr < NUM_SUB; sr++) {
                int gr = r * NUM_SUB + sr;
                for (int gi = 0; gi < gapsPerCell; gi++)
                    s += congestion[gr][c * gapsPerCell + gi];
            }
            cellCongest_OLD[r][c] = (double)s / cellGridCount;
        }
#endif // disabled old device-unit-level HierCCost Part 1

    // ============================================================
    // [Part 2] Lateral x congestion (same as original cCost)
    // ============================================================
    vector<vector<string>> unitGrid(R);
    vector<vector<int>>    unitCell(R);
    for (int r = 0; r < R; ++r)
        for (int c = 0; c < C; ++c)
            for (const auto& du : table[r][c].GetDeviceUnits()) {
                unitGrid[r].push_back(du.GetSymbol());
                unitCell[r].push_back(c);
            }

    const int W = (int)unitGrid[0].size();
    if (W <= 0) { costMap[CostEnum::hierCCost] = 0.0; return; }
    for (int r = 1; r < R; ++r)
        if ((int)unitGrid[r].size() != W) { costMap[CostEnum::hierCCost] = 0.0; return; }

    const double wH = 1.0, wV = 0.26;
    vector<vector<double>> H_local(R, vector<double>(W, 0.0));
    vector<vector<double>> V_local(R, vector<double>(W, 0.0));

    for (int r = 0; r < R; ++r)
        for (int i = 0; i < W - 1; ++i) {
            const string& a = unitGrid[r][i];
            const string& b = unitGrid[r][i + 1];
            if (is_dummy(a) || is_dummy(b)) continue;
            if (a != b) {
                H_local[r][i] += cellCongest[r][unitCell[r][i]];
                H_local[r][i + 1] += cellCongest[r][unitCell[r][i + 1]];
            }
        }

    for (int i = 0; i < W; ++i)
        for (int r = 0; r < R - 1; ++r) {
            const string& a = unitGrid[r][i];
            const string& b = unitGrid[r + 1][i];
            if (is_dummy(a) || is_dummy(b)) continue;
            if (a != b) {
                V_local[r][i] += cellCongest[r][unitCell[r][i]];
                V_local[r + 1][i] += cellCongest[r + 1][unitCell[r + 1][i]];
            }
        }

    unordered_map<string, double> type_row_sum, type_col_sum;
    unordered_map<string, long long> type_cnt;
    for (int r = 0; r < R; ++r)
        for (int i = 0; i < W; ++i) {
            const string& t = unitGrid[r][i];
            if (is_dummy(t)) continue;
            type_row_sum[t] += H_local[r][i];
            type_col_sum[t] += V_local[r][i];
            type_cnt[t]++;
        }

    double sum_avg_row = 0.0, sum_avg_col = 0.0;
    int type_num = 0;
    for (const auto& kv : type_cnt) {
        const string& t = kv.first;
        long long cnt = kv.second;
        if (cnt <= 0) continue;
        sum_avg_row += type_row_sum[t] / cnt;
        sum_avg_col += type_col_sum[t] / cnt;
        type_num++;
    }

    const double C_row = type_num > 0 ? sum_avg_row / type_num : 0.0;
    const double C_col = type_num > 0 ? sum_avg_col / type_num : 0.0;

    costMap[CostEnum::hierCCost] = wH * C_row + wV * C_col;
}

void TableManager::CalculateSymmetryCost()
{
 //   double noSymmetryScore = 0.0;
 //   for (int r = 0; r < rowSize / 2; r++)
 //   {
 //       for (int c = 0; c < colSize / 2; c++)
 //       {
 //           Group g = table[r][c];
 //           Group gSym = table[rowSize - 1 - r][colSize - 1 - c];

 //           if (g == gSym) continue;
 //           else noSymmetryScore += 1.0;
 //       }
 //   }

	//costMap[CostEnum::symmetryCost] = noSymmetryScore; // lower is better

    double noSymmetryScore = 0.0;
	unordered_map<int, unordered_map<int, int>> symmetryPairCount;
	unordered_map<int, int> groupCount;
    for (int r = 0; r < rowSize; r++)
    {
        for (int c = 0; c < colSize / 2; c++)
        {
			auto g1Hash = table[r][c].GetSymbolNameSequenceHash();
            auto g2Hash = table[r][colSize - 1 - c].GetSymbolNameSequenceHash();

			symmetryPairCount[g1Hash][g2Hash]++;
			symmetryPairCount[g2Hash][g1Hash]++;

			groupCount[g1Hash]++;
			groupCount[g2Hash]++;
        }
    }

    for (const auto& g1Main : symmetryPairCount)
    {
		int maxCount = 0;
        for (const auto& g2Slave : g1Main.second)
        {
			maxCount = std::max(maxCount, g2Slave.second);
		}

		double pairNoSymmetryScore = 1.0 - (double)maxCount / groupCount[g1Main.first];
		noSymmetryScore += pairNoSymmetryScore;
    }

    costMap[CostEnum::symmetryCost] = noSymmetryScore; // lower is better
}

pair<double, double> CalEachWindowSizeCost(TableManager table, int rowSize, int colSize, int rowWindowSize, int colWindowSize)
{
    int sameGroupCount = 0;
    int allGroupCount = 0;

    for (int i = 0; i + rowWindowSize - 1 < rowSize; i += rowWindowSize)
    {
        for (int j = 0; j + colWindowSize - 1 < colSize; j += colWindowSize)
        {
            allGroupCount++;
            Group currentGroup = table.GetGroup(i, j);

            //cout << rowSize << " " << colSize << " " << rowWindowSize << " " << colWindowSize << "\n";

            bool sameGroup = true;
            // 修正邊界：只檢查當前的 Window 範圍
            for (int x = i; x < i + rowWindowSize; ++x)
            {
                for (int y = j; y < j + colWindowSize; ++y)
                {
                    if (table.GetGroup(x, y).GetSymbolNameSequence() != currentGroup.GetSymbolNameSequence())
                    {
                        sameGroup = false;
                        break;
                    }
                }
                if (!sameGroup)
                {
                    break;
                }
            }

            if (sameGroup)
            {
                sameGroupCount++;
            }
        }
    }


    double coverageRate = (double)sameGroupCount / allGroupCount;
    int bestRowWindowSize = rowWindowSize, bestColWindowSize = colWindowSize;


    if (rowWindowSize > 1)
    {
        pair<double, double> newCoverageRate = CalEachWindowSizeCost(table, rowSize, colSize, rowWindowSize / 2, colWindowSize);
        if (newCoverageRate.first > coverageRate || (newCoverageRate.first == coverageRate && (rowWindowSize / 2 * colWindowSize > bestRowWindowSize * bestColWindowSize)))
        {
            coverageRate = newCoverageRate.first;
            bestRowWindowSize = rowWindowSize / 2;
            bestColWindowSize = colWindowSize;
        }
    }

    if (colWindowSize > 1)
    {
        pair<double, double> newCoverageRate = CalEachWindowSizeCost(table, rowSize, colSize, rowWindowSize, colWindowSize / 2);
        if (newCoverageRate.first > coverageRate || (newCoverageRate.first == coverageRate && (rowWindowSize * colWindowSize / 2 > bestRowWindowSize * bestColWindowSize)))
        {
            coverageRate = newCoverageRate.first;
            bestRowWindowSize = rowWindowSize;
            bestColWindowSize = colWindowSize / 2;
        }
    }

    return { coverageRate, (double)(bestRowWindowSize * bestColWindowSize) / (rowSize * colSize) };
}

void TableManager::CalculateWindowSizeCost()
{
    int rowWindowSize = rowSize;
    int colWindowSize = colSize;
    pair<double, double> windowSizeCost = CalEachWindowSizeCost(*this, rowSize, colSize, rowWindowSize, colWindowSize);
    if ((int)(windowSizeCost.first * 100) != 100)
    {
        costMap[CostEnum::windowSizeCost] = INT_MAX; // perfect coverage, no penalty
    }
    else
    {
        costMap[CostEnum::windowSizeCost] = 1.0 / (windowSizeCost.second * table[0][0].GetDeviceUnits().size()); // lower is better, normalized by device units
    }
}

void TableManager::FlipLeftHalf()
{
    for (int r = 0; r < rowSize; r++)
    {
        for (int c = 0; c < colSize / 2; c++)
        {
			table[r][c].FlipGroupRotation();
        }
    }
}