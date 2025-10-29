//
// Created by mschartner on 10/22/25.
//

#include "Spacecraft.h"


using namespace std;
using namespace VieVS;
namespace fs = std::filesystem;


Spacecraft::Spacecraft(
    const std::string &name,
    std::unordered_map<std::string, std::unique_ptr<AbstractFlux>> &src_flux,
    std::unordered_map<std::string, std::vector<std::tuple<unsigned int,double,double,double>>> const &EphemerisMap)
  : AbstractSource(name, name, src_flux)
{
    
    for (const auto &kv : EphemerisMap) {
        const std::string &stationID = kv.first;
        const auto &vec = kv.second;

        for (const auto &tpl : vec) {
            unsigned int t      = std::get<0>(tpl);
            double ra           = std::get<1>(tpl);
            double dec          = std::get<2>(tpl);
            double dist         = std::get<3>(tpl);

            time_list_[stationID].push_back(t);
            ra_list_[stationID].push_back(ra);
            dec_list_[stationID].push_back(dec);
            dist_list_[stationID].push_back(dist);
        }
    }
}



// --------------------- TEST FUNCTION 1. START -----------------------
void Spacecraft::printEphemSample(const std::string &spacecraft, const std::string &station, std::size_t n, std::ostream &os) const {
    auto it_t = time_list_.find(station);
    auto it_ra = ra_list_.find(station);
    auto it_dec = dec_list_.find(station);
    auto it_dist = dist_list_.find(station);

    if (it_t == time_list_.end()) {
        os << "[no time_list_ for station " << station << "]\n";
        return;
    }
    const auto &times = it_t->second;
    const auto &ras   = (it_ra  != ra_list_.end() ? it_ra->second : std::vector<double>{});
    const auto &decs  = (it_dec != dec_list_.end() ? it_dec->second : std::vector<double>{});
    const auto &dists = (it_dist!= dist_list_.end() ? it_dist->second : std::vector<double>{});

    if (times.empty()) {
        os << "[empty time list for station " << station << "]\n";
        return;
    }

    std::size_t to_print = std::min(n, times.size());
    os << "Spacecraft '" << spacecraft << "' station '" << station << "' first " << to_print << " entries:\n";
    os << std::setw(12) << "time" << std::setw(16) << "ra" << std::setw(16) << "dec" << std::setw(16) << "dist" << '\n';
    os << std::fixed << std::setprecision(8);

    for (std::size_t i = 0; i < to_print; ++i) {
        unsigned int t = times[i];
        double ra = (i < ras.size() ? ras[i] : std::numeric_limits<double>::quiet_NaN());
        double dec = (i < decs.size() ? decs[i] : std::numeric_limits<double>::quiet_NaN());
        double dist = (i < dists.size() ? dists[i] : std::numeric_limits<double>::quiet_NaN());
        os << std::setw(12) << t
           << std::setw(16) << ra
           << std::setw(16) << dec
           << std::setw(16) << dist << '\n';
    }
    os << std::flush;
}
// --------------------- TEST FUNCTION 1. END -----------------------




std::pair<double, double> Spacecraft::calcRaDe(
    unsigned int time,
    const std::shared_ptr<const Position> &sta_pos) const
{
    const std::string &stationID = sta_pos->getName();

    auto it_time = time_list_.find(stationID);
    auto it_ra   = ra_list_.find(stationID);
    auto it_dec  = dec_list_.find(stationID);

    const auto &times = it_time->second;
    const auto &ras   = it_ra->second;
    const auto &decs  = it_dec->second;

    for (size_t i = 1; i < times.size(); ++i) {
        if (time <= times[i]) {
            unsigned int t0 = times[i-1];
            unsigned int t1 = times[i];
            double ra0 = ras[i-1], ra1 = ras[i];
            double de0 = decs[i-1], de1 = decs[i];

            double alpha = static_cast<double>(time - t0) / (t1 - t0);

            double ra = ra0 + alpha * (ra1 - ra0);
            double de = de0 + alpha * (de1 - de0);
            return {ra, de};
        }
    }
    return {ras.back(), decs.back()};
}




// --------------------- TEST FUNCTION 2. START -----------------------
std::pair<double, double> Spacecraft::calcRaDe2(
    unsigned int time,
    const std::string &stationID) const
{

    auto it_time = time_list_.find(stationID);
    auto it_ra   = ra_list_.find(stationID);
    auto it_dec  = dec_list_.find(stationID);

    const auto &times = it_time->second;
    const auto &ras   = it_ra->second;
    const auto &decs  = it_dec->second;

    for (size_t i = 1; i < times.size(); ++i) {
        if (time <= times[i]) {
            unsigned int t0 = times[i-1];
            unsigned int t1 = times[i];
            double ra0 = ras[i-1], ra1 = ras[i];
            double de0 = decs[i-1], de1 = decs[i];

            double alpha = static_cast<double>(time - t0) / (t1 - t0);

            double ra = ra0 + alpha * (ra1 - ra0);
            double de = de0 + alpha * (de1 - de0);
            return {ra, de};
        }
    }
    return {ras.back(), decs.back()};
}
// --------------------- TEST FUNCTION 2. END -----------------------




std::tuple<double, double, double, double> Spacecraft::calcRaDeDistTime(
    unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const noexcept {
    const string &stationID = sta_pos->getName();

    auto it_time = time_list_.find(stationID);
    auto it_ra   = ra_list_.find(stationID);
    auto it_dec  = dec_list_.find(stationID);
    auto it_dist  = dec_list_.find(stationID);


    const auto &times = it_time->second;
    const auto &ras   = it_ra->second;
    const auto &decs  = it_dec->second;
    const auto &dists  = it_dist->second;


    for (size_t i = 1; i < times.size(); ++i) {
        if (time <= times[i]) {
            unsigned int t0 = times[i-1];
            unsigned int t1 = times[i];
            double ra0 = ras[i-1], ra1 = ras[i];
            double de0 = decs[i-1], de1 = decs[i];
            double dist0 = dists[i-1], dist1 = dists[i];


            double alpha = static_cast<double>(time - t0) / (t1 - t0);

            double ra = ra0 + alpha * (ra1 - ra0);
            double de = de0 + alpha * (de1 - de0);
            double dist = dist0 + alpha * (dist1 - dist0);
            return {ra, de, dist, time};
        }
    }
    return {ras.back(), decs.back(), dists.back(), time};
}


boost::optional<std::vector<std::tuple<unsigned int, double, double, double>>>
Spacecraft::extractEphemerisData(const std::string &folder,
                                 const std::string &name,
                                 const std::string &station) {
    namespace fs = std::filesystem;
    using boost::posix_time::ptime;
    using namespace std;

    // Define session time range (used to compute seconds since start).
    ptime session_start = TimeSystem::startTime;
    ptime session_end   = TimeSystem::endTime;

    // Open the file: <folder>/<name>_<station>.txt
    fs::path filepath = fs::path(folder) / (name + "_" + station + ".txt");
    if (!fs::exists(filepath) || !fs::is_regular_file(filepath))
        return boost::none;

    ifstream file(filepath);
    if (!file.is_open())
        return boost::none;

    string line;
    // --- Find the header line containing column names ---
    vector<string> headers;
    bool header_found = false;
    while (getline(file, line)) {
        // Remove leading/trailing whitespace for checking
        string temp = line;
        while (!temp.empty() && isspace((unsigned char)temp.front())) temp.erase(temp.begin());
        if (temp.rfind("Date__(UT)__HR:MN", 0) == 0) {
            // Found header line
            header_found = true;
            break;
        }
    }
    if (!header_found)
        return boost::none;

    // Split header line by comma into fields
    {
        istringstream iss(line);
        string cell;
        while (getline(iss, cell, ',')) {
            // Trim whitespace around header
            auto trim = [&](string &s) {
                while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
                while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
            };
            trim(cell);
            headers.push_back(cell);
        }
    }

    // Lambda to find the index of a header name (exact match)
    auto find_index = [&](const string &name) -> int {
        for (size_t i = 0; i < headers.size(); ++i) {
            if (headers[i] == name) return static_cast<int>(i);
        }
        return -1;
    };

    // Find indices for Date, RA, DEC, and delta columns
    int idx_date = find_index("Date__(UT)__HR:MN");
    int idx_ra   = find_index("R.A._(ICRF)");
    int idx_dec  = find_index("DEC__(ICRF)");
    int idx_delta = find_index("delta");

    if (idx_date < 0 || idx_ra < 0 || idx_dec < 0 || idx_delta < 0) {
        // Missing one of the required columns
        return boost::none;
    }

    // Vector to accumulate raw strings for each column
    vector<string> time_str, ra_str, dec_str;
    vector<double> dist_km;
    constexpr double AU_IN_KM = 149597870.7;

    // --- Skip lines until $$SOE (start of data) ---
    bool data_started = false;
    while (getline(file, line)) {
        // Trim leading spaces
        string trimmed = line;
        while (!trimmed.empty() && isspace((unsigned char)trimmed.front())) trimmed.erase(trimmed.begin());
        if (trimmed.rfind("$$SOE", 0) == 0) {
            data_started = true;
            break;
        }
    }
    if (!data_started) {
        // No data marker found
        return boost::none;
    }

    // --- Read data lines until $$EOE (end of data) ---
    while (getline(file, line)) {
        // Trim leading spaces for checking markers
        string trimmed = line;
        while (!trimmed.empty() && isspace((unsigned char)trimmed.front())) trimmed.erase(trimmed.begin());

        // Check for end-of-ephemeris marker
        if (trimmed.rfind("$$EOE", 0) == 0) {
            break;
        }
        // Skip other marker or blank lines
        if (trimmed.empty() || trimmed.rfind("$$", 0) == 0) continue;

        // Split the line by commas into fields
        istringstream iss(line);
        vector<string> fields;
        string cell;
        while (getline(iss, cell, ',')) {
            // Trim whitespace around each field
            auto trim = [&](string &s) {
                while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
                while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
            };
            trim(cell);
            fields.push_back(cell);
        }

        // Check that we have all needed fields
        int max_idx = max({idx_date, idx_ra, idx_dec, idx_delta});
        if ((int)fields.size() <= max_idx) {
            // Malformed line
            return boost::none;
        }

        // Extract the necessary fields
        string datetime = fields[idx_date];   // e.g. "2025-Oct-22 00:00"
        string ra_field = fields[idx_ra];     // e.g. "14 00 51.82"
        string dec_field = fields[idx_dec];   // e.g. "-16 01 29.4"

        // Parse distance (delta in AU) and convert to kilometers
        double delta_au = 0.0;
        try {
            delta_au = boost::lexical_cast<double>(fields[idx_delta]);
        } catch (...) {
            // Failed numeric conversion
            return boost::none;
        }

        // Store raw strings for conversion later
        time_str.push_back(datetime);
        ra_str.push_back(ra_field);
        dec_str.push_back(dec_field);
        dist_km.push_back(delta_au * AU_IN_KM);
    }

    if (time_str.empty()) {
        // No data parsed
        return boost::none;
    }

    // Prepare time parser for format "YYYY-Mmm-DD HH:MM"
    static const locale timeLocale(locale::classic(),
        new boost::posix_time::time_input_facet("%Y-%b-%d %H:%M"));

    vector<unsigned int> time_sec;
    vector<double> ra_deg;
    vector<double> dec_deg;
    time_sec.reserve(time_str.size());
    ra_deg.reserve(ra_str.size());
    dec_deg.reserve(dec_str.size());

    // Convert each row: time string -> seconds, RA -> degrees, DEC -> degrees
    for (size_t i = 0; i < time_str.size(); ++i) {
        // Parse time
        ptime t;
        istringstream ss(time_str[i]);
        ss.imbue(timeLocale);
        ss >> t;
        if (t.is_not_a_date_time()) {
            return boost::none;
        }
        time_sec.push_back(static_cast<unsigned int>((t - session_start).total_seconds()));

        // Parse RA (H M S -> degrees)
        {
            istringstream iss(ra_str[i]);
            string sh, sm, ss_s;
            if (!(iss >> sh >> sm >> ss_s)) return boost::none;
            try {
                double h = boost::lexical_cast<double>(sh);
                double m = boost::lexical_cast<double>(sm);
                double sec = boost::lexical_cast<double>(ss_s);
                // 15 deg per hour
                ra_deg.push_back(15.0 * (h + m/60.0 + sec/3600.0));
            } catch (...) {
                return boost::none;
            }
        }

        // Parse DEC (sign D M S -> degrees)
        {
            istringstream iss(dec_str[i]);
            string degs, dmin, dsec;
            if (!(iss >> degs >> dmin >> dsec)) return boost::none;
            char sign = '+';
            if (!degs.empty() && (degs[0] == '+' || degs[0] == '-')) {
                sign = degs[0];
                degs = degs.substr(1);
            }
            try {
                double ddeg = boost::lexical_cast<double>(degs);
                double dmn  = boost::lexical_cast<double>(dmin);
                double ds   = boost::lexical_cast<double>(dsec);
                double dec_val = fabs(ddeg) + dmn/60.0 + ds/3600.0;
                if (sign == '-') dec_val = -dec_val;
                dec_deg.push_back(dec_val);
            } catch (...) {
                return boost::none;
            }
        }
    }

    // Combine into tuples (time_sec, RA_deg, DEC_deg, distance_km)
    using Row = std::tuple<unsigned int, double, double, double>;
    vector<Row> ephemerisData;
    ephemerisData.reserve(time_sec.size());
    for (size_t i = 0; i < time_sec.size(); ++i) {
        ephemerisData.emplace_back(time_sec[i], ra_deg[i], dec_deg[i], dist_km[i]);
    }
    return ephemerisData;
}




std::pair<std::pair<double, double>, std::vector<double>> Spacecraft::getSourceInCrs(
    unsigned int time, const std::shared_ptr<const Position> &sta_pos ) const {

    auto srcRaDe = getRaDe( time, sta_pos );
    double cosDe = cos( srcRaDe.second );

    return { srcRaDe, { cosDe * cos( srcRaDe.first ), cosDe * sin( srcRaDe.first ), sin( srcRaDe.second ) } };
}



void Spacecraft::toVex( std::ofstream &of ) const {
    string eol = ";\n";
    of << "    def " << getName() << eol;
    of << "        source_type = spacecraft" << eol;
    of << "    enddef;\n";
}

void Spacecraft::toVex( std::ofstream &of, const std::vector<unsigned int> &times,
                        const std::shared_ptr<const Position> &sta_pos ) const {
    string eol = ";\n";
    for ( unsigned int t : times ) {
        string name = getNameTime( t );
        auto rade = getRaDe( t, sta_pos );

        of << "    def " << name << eol;
        of << "        source_type = star" << eol;
        of << "        source_name = " << name << eol;
        of << "        ra = " << getRaString( rade.first ) << eol;
        of << "        dec = " << getDeString( rade.second ) << eol;
        of << "        ref_coord_frame = J2000" << eol;
        of << "        ra_rate = 0 asec/yr" << eol;
        of << "        dec_rate = 0 asec/yr" << eol;
        of << "    enddef;\n";
    }
}
void Spacecraft::toNgsHeader( std::ofstream &of ) const {
    string name = getName();
    of << "spacecraft " << getName() << "\n";
}

