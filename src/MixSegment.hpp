#ifndef CPPJIEBA_MIXSEGMENT_H
#define CPPJIEBA_MIXSEGMENT_H

#include <cassert>
#include "MPSegment.hpp"
#include "HMMSegment.hpp"
#include "Limonp/StringUtil.hpp"

namespace CppJieba {
class MixSegment: public SegmentBase {
 public:
  MixSegment(const string& mpSegDict, const string& hmmSegDict, 
        const string& userDict = "") 
    : mpSeg_(mpSegDict, userDict), 
      hmmSeg_(hmmSegDict) {
    LogInfo("MixSegment init %s, %s", mpSegDict.c_str(), hmmSegDict.c_str());
  }
  virtual ~MixSegment() {
  }
  using SegmentBase::cut;
  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<Unicode>& res) const {
    vector<Unicode> words;
    words.reserve(end - begin);
    if(!mpSeg_.cut(begin, end, words)) {
      LogError("mpSeg cutDAG failed.");
      return false;
    }

    vector<Unicode> hmmRes;
    hmmRes.reserve(end - begin);
    Unicode piece;
    piece.reserve(end - begin);
    for (size_t i = 0, j = 0; i < words.size(); i++) {
      //if mp get a word, it's ok, put it into result
      if (1 != words[i].size() || (words[i].size() == 1 && mpSeg_.isUserDictSingleChineseWord(words[i][0]))) {
        res.push_back(words[i]);
        continue;
      }

      // if mp get a single one and it is not in userdict, collect it in sequence
      j = i;
      while (j < words.size() && 1 == words[j].size() && !mpSeg_.isUserDictSingleChineseWord(words[j][0])) {
        piece.push_back(words[j][0]);
        j++;
      }

      // cut the sequence with hmm
      if (!hmmSeg_.cut(piece.begin(), piece.end(), hmmRes)) {
        LogError("hmmSeg_ cut failed.");
        return false;
      }

      //put hmm result to result
      for (size_t k = 0; k < hmmRes.size(); k++) {
        res.push_back(hmmRes[k]);
      }

      //clear tmp vars
      piece.clear();
      hmmRes.clear();

      //let i jump over this piece
      i = j - 1;
    }
    return true;
  }

  virtual bool cut(Unicode::const_iterator begin, Unicode::const_iterator end, vector<string>& res)const {
    if(begin == end) {
      return false;
    }

    vector<Unicode> uRes;
    uRes.reserve(end - begin);
    if (!cut(begin, end, uRes)) {
      return false;
    }

    size_t offset = res.size();
    res.resize(res.size() + uRes.size());
    for(size_t i = 0; i < uRes.size(); i ++, offset++) {
      TransCode::encode(uRes[i], res[offset]);
    }
    return true;
  }

  const DictTrie* getDictTrie() const {
    return mpSeg_.getDictTrie();
  }
 private:
  MPSegment mpSeg_;
  HMMSegment hmmSeg_;
};
}

#endif
