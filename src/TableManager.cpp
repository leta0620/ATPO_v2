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

// ---- CC (Center Closure): compute £g for each device name and average their |£g| ----
void TableManager::CalculateCCCost() {
    // For each row: flatten the row into unit-cell tokens (same logic as GetTableStringFormat)
    std::unordered_map<std::string, long long> sum_x;   // £U x for each symbol
    std::unordered_map<std::string, long long> cnt_x;   // count for each symbol

    for (int r = 0; r < rowSize; ++r) {
        // Flatten row into tokens
        std::vector<std::string> rowTok;
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
            const std::string& name = rowTok[i];
            if (name.empty() || name == "d") continue;

            long long x = even
                ? ((i < k) ? (i - k) : (i - k + 1))
                : (i - m);

            sum_x[name] += x;
            cnt_x[name] += 1;
        }
    }

    // Compute £g for each name and average their absolute values |£g|
    double sum_abs_mu = 0.0;
    int n = 0;
    for (const auto& kv : sum_x) {
        const std::string& name = kv.first;
        long long c = cnt_x[name];
        if (c <= 0) continue;

        double mu = static_cast<double>(kv.second) / static_cast<double>(c);
        sum_abs_mu += std::fabs(mu);   //ABS!!!
        ++n;
    }

    costMap[CostEnum::ccCost] = (n == 0) ? 0.0 : (sum_abs_mu / static_cast<double>(n));
}


// ---- R (Row Spread): column distance ¡Ñ unit count ¡÷ per-name mean ¡÷ population stddev ----
void TableManager::CalculateRCost() {
    std::unordered_map<std::string, double> sum_wdist;  // £U(dist ¡Ñ count)
    std::unordered_map<std::string, long long> sum_cnt; // total count

    const bool even = (colSize % 2 == 0);
    const int  k = colSize / 2;
    const int  m = (colSize - 1) / 2;

    auto col_dist = [&](int j)->double {
        return even ? std::fabs((static_cast<double>(j) + 0.5) - static_cast<double>(k))
            : std::fabs(static_cast<double>(j - m));
        };

    for (int r = 0; r < rowSize; ++r) {
        for (int j = 0; j < colSize; ++j) {
            const auto counts = CountNameInGroup(table[r][j]);
            const double d = col_dist(j);
            for (const auto& kv : counts) {
                sum_wdist[kv.first] += d * static_cast<double>(kv.second);
                sum_cnt[kv.first] += kv.second;
            }
        }
    }

    // Compute average distance per name
    std::vector<double> per_name_avg;
    per_name_avg.reserve(sum_wdist.size());
    for (const auto& kv : sum_wdist) {
        const std::string& name = kv.first;
        long long c = sum_cnt[name];
        if (c <= 0) continue;
        per_name_avg.push_back(kv.second / static_cast<double>(c));
    }

    // Compute population standard deviation
    double r_cost = 0.0;
    if (!per_name_avg.empty()) {
        double s = 0.0, s2 = 0.0;
        for (double v : per_name_avg) { s += v; s2 += v * v; }
        double n = static_cast<double>(per_name_avg.size());
        double mean = s / n;
        double var = (s2 / n) - mean * mean;
        if (var < 0.0) var = 0.0;
        r_cost = std::sqrt(var);
    }

    costMap[CostEnum::rCost] = r_cost;
}

// ---- C (Column Closure): both sides from outside to inside; add penalty when new name appears ----
void TableManager::CalculateCCost() {
    const int nfin_local = std::max(2, this->nfin); // avoid dividing by zero

    auto process_side = [&](int r, int start, int end, int step,
        std::unordered_map<std::string, double>& sum_w,
        std::unordered_map<std::string, long long>& sum_cnt) {

            std::unordered_set<std::string> seen;
            std::string prev;
            int numerator = 1;

            for (int j = start; j != end + step; j += step) {

                // Get main symbol of this group
                std::string gname = ExtractGroupName(table[r][j]);
                if (gname.empty()) continue;

                // Penalty: new symbol that has not appeared on this side
                if (!prev.empty() && gname != prev && !seen.count(gname)) {
                    numerator += 1;
                }
                prev = gname;
                seen.insert(gname);

                // Add weighted count for all units in this group
                auto counts = CountNameInGroup(table[r][j]);
                for (const auto& kv : counts) {
                    const std::string& name = kv.first;
                    int cnt = kv.second;

                    sum_w[name] += (static_cast<double>(numerator) / static_cast<double>(nfin_local - 1))
                        * static_cast<double>(cnt);

                    sum_cnt[name] += cnt;
                }
            }
        };

    std::unordered_map<std::string, double> sum_w;
    std::unordered_map<std::string, long long> sum_cnt;

    for (int r = 0; r < rowSize; ++r) {
        if (colSize == 0) continue;

        if (colSize % 2 == 0) {
            int k = colSize / 2;
            if (k - 1 >= 0) process_side(r, 0, k - 1, +1, sum_w, sum_cnt);      // left: outer ¡÷ inner
            if (colSize - 1 >= k) process_side(r, colSize - 1, k, -1, sum_w, sum_cnt);  // right: outer ¡÷ inner
        }
        else {
            int m = (colSize - 1) / 2;
            process_side(r, 0, m, +1, sum_w, sum_cnt);                                 // left (includes center)
            if (colSize - 1 >= m + 1) process_side(r, colSize - 1, m + 1, -1, sum_w, sum_cnt);
        }
    }

    double c_total = 0.0;
    for (const auto& kv : sum_w) {
        const std::string& name = kv.first;
        long long tot = sum_cnt[name];
        if (tot <= 0) continue;
        c_total += kv.second / static_cast<double>(tot);
    }

    costMap[CostEnum::cCost] = c_total;
}

// ---- Separation: unit-cell based metric using rho(distance), then compute pairwise £m_ij ----
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

    // Compute £U £m_ij across all pairs
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

    costMap[CostEnum::sperationCost] = total_sigma;
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