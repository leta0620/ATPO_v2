// cost part implementation
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

#include "TableManager.h"

using namespace std;

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

TableManager::TableManager(int groupSize, int rowSize, int colSize, NetlistLookupTable& netlist)
{
    this->groupSize = groupSize;
    this->rowSize = rowSize;
    this->colSize = colSize;
    this->netlist = netlist;
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
    this->CalculateCCCost();
    this->CalculateRCost();
    this->CalculateCCost();
    this->CalculateSpetationCost();
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

    const int R = rowSize;
    const int C = colSize;
    if (R <= 0 || C <= 0) {
        costMap[CostEnum::cCost] = 0.0;
        return;
    }

    auto is_dummy = [](const string& s) -> bool {
        if (s.empty()) return true;
        if (s == "d" || s == "D") return true;
        return false;
        };

    // -----------------------------
    // Step0: build flattened grid
    // grid[r][i] = symbol of i-th device unit in row r
    // -----------------------------
    vector<vector<string>> grid(R);
    for (int r = 0; r < R; ++r) {
        for (int c = 0; c < C; ++c) {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units) {
                grid[r].push_back(du.GetSymbol());
            }
        }
    }

    const int W = static_cast<int>(grid[0].size());
    if (W <= 0) {
        costMap[CostEnum::cCost] = 0.0;
        return;
    }

    // Ensure rectangular
    for (int r = 1; r < R; ++r) {
        if ((int)grid[r].size() != W) {
            costMap[CostEnum::cCost] = 0.0;
            return;
        }
    }

    // -----------------------------
    // Example 1 weights (your choice)
    // Make P2 and P3 close but ordered
    // -----------------------------
    const double wH = 1.0;
    const double wV = 0.26;

    // -----------------------------
    // Step1: per-device local counts
    // H_local / V_local (endpoints +1)
    // -----------------------------
    vector<vector<int>> H_local(R, vector<int>(W, 0));
    vector<vector<int>> V_local(R, vector<int>(W, 0));

    // Horizontal transitions
    for (int r = 0; r < R; ++r) {
        for (int i = 0; i < W - 1; ++i) {
            const string& a = grid[r][i];
            const string& b = grid[r][i + 1];
            if (is_dummy(a) || is_dummy(b)) continue;
            if (a != b) {
                H_local[r][i] += 1;
                H_local[r][i + 1] += 1;
            }
        }
    }

    // Vertical transitions (COLUMN direction)
    for (int i = 0; i < W; ++i) {
        for (int r = 0; r < R - 1; ++r) {
            const string& a = grid[r][i];
            const string& b = grid[r + 1][i];
            if (is_dummy(a) || is_dummy(b)) continue;
            if (a != b) {
                V_local[r][i] += 1;
                V_local[r + 1][i] += 1;
            }
        }
    }

    // -----------------------------
    // Step2: per-type accumulation
    // row_cost(u)=H_local(u), col_cost(u)=V_local(u)
    // -----------------------------
    std::unordered_map<string, double> type_row_sum; // �U H_local(u)
    std::unordered_map<string, double> type_col_sum; // �U V_local(u)
    std::unordered_map<string, long long> type_cnt;  // N_type

    for (int r = 0; r < R; ++r) {
        for (int i = 0; i < W; ++i) {
            const string& t = grid[r][i];
            if (is_dummy(t)) continue;

            type_row_sum[t] += (double)H_local[r][i];
            type_col_sum[t] += (double)V_local[r][i];
            type_cnt[t] += 1;
        }
    }

    // -----------------------------
    // Step3: per-type average, then average over types
    // C_row = avg_over_types(avg_row(type))
    // C_col = avg_over_types(avg_col(type))
    // final: Ccost = wH*C_row + wV*C_col
    // -----------------------------
    double sum_type_avg_row = 0.0;
    double sum_type_avg_col = 0.0;
    int type_num = 0;

    for (const auto& kv : type_cnt) {
        const string& t = kv.first;
        const long long cnt = kv.second;
        if (cnt <= 0) continue;

        const double avg_row_t = type_row_sum[t] / (double)cnt;
        const double avg_col_t = type_col_sum[t] / (double)cnt;

        sum_type_avg_row += avg_row_t;
        sum_type_avg_col += avg_col_t;
        type_num += 1;
    }

    const double C_row = (type_num > 0) ? (sum_type_avg_row / (double)type_num) : 0.0;
    const double C_col = (type_num > 0) ? (sum_type_avg_col / (double)type_num) : 0.0;

    const double Ccost = wH * C_row + wV * C_col;
    costMap[CostEnum::cCost] = Ccost;
}



// ---- Separation: unit-cell based metric using rho(distance), then compute pairwise �m_ij ----
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
        std::string costName;
        switch (costEnum)
        {
        case CostEnum::ccCost:
            costName = "System variation";
            break;
        case CostEnum::rCost:
            costName = "Routing Length";
            break;
        case CostEnum::cCost:
            costName = "Coupling capacitance";
            break;
        case CostEnum::sperationCost:
            costName = "Dispersion";
            break;
        default:
            costName = "Unknown Cost";
            break;
        }
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
        return "Routing Length";
    }
    else if (costEnum == CostEnum::cCost)
    {
        return "Coupling capacitance";
    }
    else if (costEnum == CostEnum::sperationCost)
    {
        return "Dispersion";
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
                interleavingTable.push_back(rowGroups);
				count = 0;
            }
		}
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


    //this->table = interleavingTable;

    //// 原有的 interleaving 建構邏輯被註解掉，保留列印 table 的行為方便除錯/檢查。
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

}