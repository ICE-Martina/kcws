/*
 * Copyright 2016- 2018 Koth. All Rights Reserved.
 * =====================================================================================
 * Filename:  viterbi_decode.cc
 * Author:  Koth
 * Create Time: 2017-02-01 13:47:48
 * Description:
 *
 */
#include "kcws/cc/viterbi_decode.h"

namespace kcws {
// viterbi算法，考虑到了自定义词
int viterbi_decode(
  const Eigen::TensorMap<Eigen::Tensor<float, 3, Eigen::RowMajor>, Eigen::Aligned>& predictions,
  int sentenceIdx,
  int nn,
  const std::vector<std::vector<float>>& trans,
  int** bp,
  float** scores, // scores[0]记录上一层最大得分，scores[1]记录本层最大得分
  int ntags) {
  for (int i = 0; i < ntags; i++) {
    scores[0][i] = predictions(sentenceIdx, 0, i); 
  }
  for (int i = 1; i < nn; i++) {
    for (int  t = 0; t < ntags; t++) {
      float maxScore = -1e7;
      float emission = predictions(sentenceIdx, i, t); // 第i个字，第t个标签的取值
      for (int prev = 0; prev < ntags; prev++) { // 求上一层的最大得分
        float score = scores[(i - 1) % 2][prev] + trans[prev][t] + emission;
        if (score > maxScore) {
          maxScore = score;
          bp[i - 1][t] = prev;
        }
      }
      scores[i % 2][t] = maxScore;
    }
  }
  float maxScore = scores[(nn - 1) % 2][0];
  int ret = 0;
  for (int i = 1; i < ntags; i++) {
    if (scores[(nn - 1) % 2][i] > maxScore) {
      ret = i;
      maxScore = scores[(nn - 1) % 2][i];
    }
  }
  return ret;
}
void get_best_path(
  const Eigen::TensorMap<Eigen::Tensor<float, 3, Eigen::RowMajor>, Eigen::Aligned>& predictions,
  int sentenceIdx,
  int nn,
  const std::vector<std::vector<float>>& trans,
  int** bp,
  float** scores,
  std::vector<int>& resultTags,
  int ntags) {
  int lastTag = viterbi_decode(predictions, sentenceIdx, nn, trans, bp, scores, ntags);
  resultTags.push_back(lastTag);
  for (int i = nn - 2; i >= 0; i--) {
    int bpTag = bp[i][lastTag];
    resultTags.push_back(bpTag);
    lastTag = bpTag;
  }
}

}  // namespace kcws
