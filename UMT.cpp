/*
* Assumptions:
*   - the given calendars are sorted in ascending order;
*   - there is no missing data in the input;
*   - the input data is correct and it has the correct format;
*   - each calendar contains at least one interval;
* Explanation:
*   - first I parse the input data creating a data structure with all the 
      intervals;
*   - then I merge the given calendars into one, keeping it in ascending order 
      by interval start time;
*   - after that I merge all overlapping intervals from that calendar;
*   - and finally I go over the non-overlapping calendar and find all slots 
      that are in between the given limits and are also less than the given 
      time limit;
*/


#include <vector>
#include <iostream>
#include <fstream>
#include <list>
#include <algorithm>
#include <functional>
#include <string>
#include <regex>

using namespace std;

class Time {
public:
    int hours, minutes;

    Time() : hours{ 0 }, minutes{ 0 } {}
    Time(int hours, int minutes) : hours{ hours }, minutes{ minutes } {}

    string toString() {
        return "'" + to_string(this->hours) + ":" + (this->minutes == 0 ? "00" : to_string(this->minutes)) + "'";
    }

    bool operator<(const Time& time)
    {
        if (this->hours < time.hours) return true;
        if (this->hours == time.hours && this->minutes < time.minutes) return true;
        return false;
    }
    
    bool operator>(const Time& time)
    {
        if (this->hours > time.hours) return true;
        if (this->hours == time.hours && this->minutes > time.minutes) return true;
        return false;
    }
    
    bool operator>=(const Time& time)
    {
        if (this->hours > time.hours) return true;
        if (this->hours == time.hours && this->minutes >= time.minutes) return true;
        return false;
    }

    int operator-(const Time& time) 
    {
        return (this->hours - time.hours) * 60 + (this->minutes - time.minutes);
    }
};

vector<pair<Time, Time>> parseCalendar(string calendar) {
    regex patternInterval(R"(\[['\"](\d{1,2}):(\d{2})['\"],\s*['\"](\d{1,2}):(\d{2})['\"]\])");
    smatch matchesInterval;

    vector<pair<Time, Time>> intervals;

    while (regex_search(calendar, matchesInterval, patternInterval)) {
        int hours1 = stoi(matchesInterval[1].str());
        int minutes1 = stoi(matchesInterval[2].str());
        int hours2 = stoi(matchesInterval[3].str());
        int minutes2 = stoi(matchesInterval[4].str());

        Time begin(hours1, minutes1);
        Time end(hours2, minutes2);
        pair<Time, Time> interval{ begin, end };
        intervals.push_back(interval);

        calendar = matchesInterval.suffix().str();
    }

    return intervals;
}

vector<pair<Time, Time>> mergeCalendars(vector<pair<Time, Time>> intervals1, vector<pair<Time, Time>> intervals2) {
    int index1 = 0, index2 = 0;
    vector<pair<Time, Time>> mergedIntervals;
    while (index1 < intervals1.size() && index2 < intervals2.size()) {
        auto [begin1, end1] = intervals1[index1];
        auto [begin2, end2] = intervals2[index2];
        if (begin1 < begin2) {
            mergedIntervals.push_back(intervals1[index1]);
            ++index1;
        }
        else {
            mergedIntervals.push_back(intervals2[index2]);
            ++index2;
        }
    }

    while (index1 < intervals1.size()) {
        mergedIntervals.push_back(intervals1[index1]);
        ++index1;
    }

    while (index2 < intervals2.size()) {
        mergedIntervals.push_back(intervals2[index2]);
        ++index2;
    }

    return mergedIntervals;
}

vector<pair<Time, Time>> mergeIntervalsInCalendar(vector<pair<Time, Time>> calendar) {
    int index = 0;
    vector<pair<Time, Time>> finalCalendar;
    while (index < calendar.size()) {
        auto [begin, end] = calendar[index];
        ++index;
        while (index < calendar.size() && end >= calendar[index].first) { ++index; }

        Time maxEnd = (end > calendar[index - 1].second ? end : calendar[index - 1].second);
        pair<Time, Time> newInterval{ begin, maxEnd };
        finalCalendar.push_back(newInterval);
    }

    return finalCalendar;
}

vector<pair<Time, Time>> findPossibleSlots(vector<pair<Time, Time>> calendar, int minutes, pair<Time, Time> lim1, pair<Time, Time> lim2) {
    // Find the time limits of both calendars
    Time maxLowerLimit = (lim1.first > lim2.first ? lim1.first : lim2.first);
    Time minUpperLimit = (lim1.second < lim2.second ? lim1.second : lim2.second);

    vector<pair<Time, Time>> answer;

    if (maxLowerLimit < calendar[0].first) {
        answer.push_back({ maxLowerLimit, calendar[0].first });
    }

    for (int i = 1; i < calendar.size(); ++i) {
        auto end = calendar[i - 1].second, start = calendar[i].first;
        if (start - end >= minutes) {
            Time lower = (maxLowerLimit > end ? maxLowerLimit : end);
            Time upper = (minUpperLimit < start ? minUpperLimit : start);
            answer.push_back({ lower, upper });
        }
    }

    if (minUpperLimit > calendar[calendar.size() - 1].second) {
        answer.push_back({ calendar[calendar.size() - 1].second, minUpperLimit });
    }

    return answer;
}

void printCalendar(vector<pair<Time, Time>> calendar) {
    ofstream out("output.txt");

    out << "[";
    for (pair<Time, Time> time : calendar) {
        out << "[" << time.first.toString() << "," << time.second.toString() << "], ";
    }
    out << "]";

    out.close();
}

int main() {
    ifstream input("input.txt");
    string calendar1, calendar2, limit1, limit2;
    int minutes;

    // Reading input
    getline(input, calendar1, '\n');
    getline(input, limit1, '\n');
    getline(input, calendar2, '\n');
    getline(input, limit2, '\n');
    input >> minutes;

    input.close();
    
    // Parsing calendars
    vector<pair<Time, Time>> intervals1 = parseCalendar(calendar1);
    vector<pair<Time, Time>> intervals2 = parseCalendar(calendar2);
    pair<Time, Time> lim1 = parseCalendar(limit1)[0];
    pair<Time, Time> lim2 = parseCalendar(limit2)[0];

    // Merging the two calendars into one ordered calendar
    vector<pair<Time, Time>> mergedCalendars = mergeCalendars(intervals1, intervals2);

    // Merging all overlapping intervals from the calendar
    vector<pair<Time, Time>> finalCalendar = mergeIntervalsInCalendar(mergedCalendars);

    // Finding all possible available slots
    vector<pair<Time, Time>> answer = findPossibleSlots(finalCalendar, minutes, lim1, lim2);

    // Printing those slots
    printCalendar(answer);

    return 0;
}
