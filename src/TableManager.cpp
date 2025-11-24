#include "TableManager.h"
// cost part implementation

#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <string>
#include <vector>
#include <algorithm>

// Utility: Return the first non-"d" symbol in a group; return empty string if all are "d"
static std::string ExtractGroupName(const Group& g) {
    for (const auto& du : g.GetDeviceUnits()) {
        const std::string& s = du.GetSymbol();
        if (!s.empty() && s != "d") return s;
    }
    return {};
}

// Utility: Count the number of units per symbol within a group (excluding "d")
static std::unordered_map<std::string, int> CountNameInGroup(const Group& g) {
    std::unordered_map<std::string, int> cnt;
    for (const auto& du : g.GetDeviceUnits()) {
        const std::string& s = du.GetSymbol();
        if (!s.empty() && s != "d") cnt[s] += 1;
    }
    return cnt;
}

// ---- CC (Center symmetry: compute mean £g per name) ----
void TableManager::CalculateCCCost() {
    // For each row, flatten into unit-cell tokens (same logic as GetTableStringFormat)
    // Collect symbol positions using the flattened index
    std::unordered_map<std::string, long long> sum_x;   // £U x per symbol
    std::unordered_map<std::string, long long> cnt_x;   // Count per symbolDD

    for (int r = 0; r < rowSize; ++r) {
        // Flatten row into tokens
        std::vector<std::string> rowTok;
        for (int c = 0; c < colSize; ++c) {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units) rowTok.push_back(du.GetSymbol());
        }
        const int W = static_cast<int>(rowTok.size());
        if (W == 0) continue;

        const bool even = (W % 2 == 0);
        const int  k = W / 2;
        const int  m = (W - 1) / 2;

        for (int i = 0; i < W; ++i) {
            const std::string& name = rowTok[i];
            if (name.empty() || name == "d") continue;
            long long x = even ? ((i < k) ? (i - k) : (i - k + 1))
                : (i - m);
            sum_x[name] += x;
            cnt_x[name] += 1;
        }
    }

    // Compute £g per symbol and take the mean (no absolute value)
    double sum_mu = 0.0;
    int n = 0;
    for (const auto& kv : sum_x) {
        const auto& name = kv.first;
        long long c = cnt_x[name];
        if (c <= 0) continue;
        double mu = static_cast<double>(kv.second) / static_cast<double>(c);
        sum_mu += mu;
        ++n;
    }

    costMap[CostEnum::ccCost] = (n == 0) ? 0.0 : (sum_mu / static_cast<double>(n));
}

// ---- R (Column-based groups; weighted distance ¡Ñ unit count; population stddev of symbol averages) ----
void TableManager::CalculateRCost() {
    std::unordered_map<std::string, double> sum_wdist;   // £U(dist * count) per symbol
    std::unordered_map<std::string, long long> sum_cnt;  // £U count per symbol

    const bool even = (colSize % 2 == 0);
    const int  k = colSize / 2;
    const int  m = (colSize - 1) / 2;

    auto col_dist = [&](int j)->double {
        return even ? std::fabs((static_cast<double>(j) + 0.5) - static_cast<double>(k))
            : std::fabs(static_cast<double>(j - m));
        };

    for (int r = 0; r < rowSize; ++r) {
        for (int j = 0; j < colSize; ++j) {
            const auto counts = CountNameInGroup(table[r][j]); // unit count per symbol in this cell
            const double d = col_dist(j);
            for (const auto& kv : counts) {
                sum_wdist[kv.first] += d * static_cast<double>(kv.second);
                sum_cnt[kv.first] += kv.second;
            }
        }
    }

    // Average distance per symbol
    std::vector<double> per_name_avg;
    per_name_avg.reserve(sum_wdist.size());
    for (const auto& kv : sum_wdist) {
        const auto& name = kv.first;
        long long c = sum_cnt[name];
        if (c <= 0) continue;
        per_name_avg.push_back(kv.second / static_cast<double>(c));
    }

    // Population standard deviation
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

// ---- C (Left/right separately; outward ¡÷ inward; increment on new symbol not previously seen on that side;
// weight = numerator/(nfin-1) ¡Ñ unit count; per-symbol average then summed) ----
void TableManager::CalculateCCost() {
    const int nfin_local = std::max(2, this->nfin); // avoid division by 0
    auto process_side = [&](int r, int start, int end, int step,
        std::unordered_map<std::string, double>& sum_w,
        std::unordered_map<std::string, long long>& sum_cnt) {
            std::unordered_set<std::string> seen;
            std::string prev;
            int numerator = 1;
            for (int j = start; j != end + step; j += step) {
                // Extract main symbol of this group (first non-"d"); skip if none
                std::string gname = ExtractGroupName(table[r][j]);
                if (gname.empty()) continue;

                if (!prev.empty() && gname != prev && !seen.count(gname)) {
                    numerator += 1;
                }
                prev = gname;
                seen.insert(gname);

                // Add all unit counts of symbols in this group (typically only one symbol plus "d")
                auto counts = CountNameInGroup(table[r][j]);
                for (const auto& kv : counts) {
                    const std::string& name = kv.first;
                    int cnt = kv.second;
                    sum_w[name] += (static_cast<double>(numerator) /
                        static_cast<double>(nfin_local - 1)) * static_cast<double>(cnt);
                    sum_cnt[name] += cnt;
                }
            }
        };

    std::unordered_map<std::string, double> sum_w;      // £U weights per symbol
    std::unordered_map<std::string, long long> sum_cnt; // £U unit count per symbol

    for (int r = 0; r < rowSize; ++r) {
        if (colSize == 0) continue;
        if (colSize % 2 == 0) {
            int k = colSize / 2;
            if (k - 1 >= 0) process_side(r, 0, k - 1, +1, sum_w, sum_cnt);               // Left: outward ¡÷ inward
            if (colSize - 1 >= k) process_side(r, colSize - 1, k, -1, sum_w, sum_cnt);  // Right: outward ¡÷ inward
        }
        else {
            int m = (colSize - 1) / 2;
            process_side(r, 0, m, +1, sum_w, sum_cnt);                                  // Left including center
            if (colSize - 1 >= m + 1) process_side(r, colSize - 1, m + 1, -1, sum_w, sum_cnt); // Right excluding center
        }
    }

    double c_total = 0.0;
    for (const auto& kv : sum_w) {
        const std::string& name = kv.first;
        long long tot = sum_cnt[name];
        if (tot <= 0) continue;
        c_total += kv.second / static_cast<double>(tot); // average per symbol then sum
    }
    costMap[CostEnum::cCost] = c_total;
}

// ---- Separation (unit-cell coordinates, non-centered; £l(d)=rho_u^d; £U £m_ij) ----
void TableManager::CalculateSpetationCost() {
    struct Pt { int r, x; }; // y=r, x=flattened unit index
    std::unordered_map<std::string, std::vector<Pt>> cells;

    // Flatten into unit-cells (ignore "d")
    for (int r = 0; r < rowSize; ++r) {
        int x = 0;
        for (int c = 0; c < colSize; ++c) {
            const auto& units = table[r][c].GetDeviceUnits();
            for (const auto& du : units) {
                const std::string& s = du.GetSymbol();
                if (!s.empty() && s != "d") cells[s].push_back(Pt{ r, x });
                x += 1; // advance regardless of "d", as it occupies space
            }
        }
    }
    if (cells.size() < 2) { costMap[CostEnum::spetationCost] = 0.0; return; }

    const double rho_u = 0.5; // tunable
    const double l_r = 1.0;
    const double l_c = 1.0;

    auto rho_of = [&](const Pt& a, const Pt& b)->double {
        const double dy = (a.r - b.r) * l_r;
        const double dx = (a.x - b.x) * l_c;
        const double d = std::sqrt(dy * dy + dx * dx);
        return std::pow(rho_u, d);
        };

    // Compute X_k
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

    // Compute £U £m_ij
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

    costMap[CostEnum::spetationCost] = total_sigma;
}
