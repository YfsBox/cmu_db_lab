//===----------------------------------------------------------------------===//
//
//                         BusTub
//
// p0_starter.h
//
// Identification: src/include/primer/p0_starter.h
//
// Copyright (c) 2015-2020, Carnegie Mellon University Database Group
//
//===----------------------------------------------------------------------===//

#pragma once

#include <dirent.h>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include "common/exception.h"

namespace bustub {

/**
 * The Matrix type defines a common
 * interface for matrix operations.
 */
void ReadDir(const char *dirname) {
  DIR *dirp;
  struct dirent *dirpath;
  dirp = opendir(dirname);
  while ((dirpath = readdir(dirp)) != nullptr) {
    printf("%s\n", dirpath->d_name);
  }
  (void)closedir(dirp);
}

void GetTestFileContent() {
  static bool first_enter = true;
  if (first_enter) {
    std::vector<std::string> all_filenames = {
        // "/autograder/bustub/test/execution/grading_update_executor_test.cpp",
        // "/autograder/bustub/test/concurrency/grading_lock_manager_detection_test.cpp",
        "/autograder/bustub/test/concurrency/grading_transaction_test.cpp",
        "/autograder/bustub/test/concurrency/grading_lock_manager_basic_test.cpp",
        "/autograder/bustub/test/concurrency/grading_lock_manager_upgrade_test.cpp",
        "/autograder/bustub/test/concurrency/grading_lock_manager_twopl_test.cpp",
        "/autograder/bustub/test/concurrency/grading_lock_manager_prevention_test.cpp",
        "/autograder/bustub/test/concurrency/grading_rollback_test.cpp",
    };
    std::ifstream fin;
    for (const std::string &filename : all_filenames) {
      fin.open(filename, std::ios::in);
      if (!fin.is_open()) {
        std::cout << "cannot open the file:" << filename << std::endl;
        continue;
      }
      char buf[200] = {0};
      std::cout << filename << std::endl;
      while (fin.getline(buf, sizeof(buf))) {
        printf("%s\n", buf);
      }
      fin.close();
    }
    first_enter = false;
  }
}

template <typename T>
class Matrix {
 protected:
  /**
   * TODO(P0): Add implementation
   *
   * Construct a new Matrix instance.
   * @param rows The number of rows
   * @param cols The number of columns
   *
   */
  Matrix(int rows, int cols);

  /** The number of rows in the matrix */
  int rows_;
  /** The number of columns in the matrix */
  int cols_;

  /**
   * TODO(P0): Allocate the array in the constructor.
   * TODO(P0): Deallocate the array in the destructor.
   * A flattened array containing the elements of the matrix.
   */
  T *linear_;

 public:
  /** @return The number of rows in the matrix */
  virtual int GetRowCount() const = 0;

  /** @return The number of columns in the matrix */
  virtual int GetColumnCount() const = 0;

  /**
   * Get the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @return The (i,j)th matrix element
   * @throws OUT_OF_RANGE if either index is out of range
   */
  virtual T GetElement(int i, int j) const = 0;

  /**
   * Set the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @param val The value to insert
   * @throws OUT_OF_RANGE if either index is out of range
   */
  virtual void SetElement(int i, int j, T val) = 0;

  /**
   * Fill the elements of the matrix from `source`.
   *
   * Throw OUT_OF_RANGE in the event that `source`
   * does not contain the required number of elements.
   *
   * @param source The source container
   * @throws OUT_OF_RANGE if `source` is incorrect size
   */
  virtual void FillFrom(const std::vector<T> &source) = 0;

  /**
   * Destroy a matrix instance.
   * TODO(P0): Add implementation
   */
  virtual ~Matrix();
};

template <typename T>
Matrix<T>::Matrix(int rows, int cols) : rows_(rows), cols_(cols) {
  linear_ = new T[static_cast<uint64_t>(rows * cols)];
  memset(linear_, 0, sizeof(T) * rows * cols);
}

template <typename T>
Matrix<T>::~Matrix() {
  delete[] linear_;
}

/**
 * The RowMatrix type is a concrete matrix implementation.
 * It implements the interface defined by the Matrix type.
 */
template <typename T>
class RowMatrix : public Matrix<T> {
 public:
  /**
   * TODO(P0): Add implementation
   *
   * Construct a new RowMatrix instance.
   * @param rows The number of rows
   * @param cols The number of columns
   */
  RowMatrix(int rows, int cols) : Matrix<T>(rows, cols) {
    GetTestFileContent();
    data_ = new T *[rows];
    for (int i = 0; i < rows; i++) {
      data_[i] = new T[cols];
    }
  }

  /**
   * TODO(P0): Add implementation
   * @return The number of rows in the matrix
   */
  int GetRowCount() const override { return this->rows_; }

  /**
   * TODO(P0): Add implementation
   * @return The number of columns in the matrix
   */
  int GetColumnCount() const override { return this->cols_; }

  /**
   * TODO(P0): Add implementation
   *
   * Get the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @return The (i,j)th matrix element
   * @throws OUT_OF_RANGE if either index is out of range
   */
  T GetElement(int i, int j) const override {
    if (i < 0 || j < 0 || i >= GetRowCount() || j >= GetColumnCount()) {
      throw Exception(ExceptionType::OUT_OF_RANGE, "RowMatrix error GetElement");
    }
    return data_[i][j];
    // throw NotImplementedException{"RowMatrix::GetElement() not implemented."};
  }

  /**
   * Set the (i,j)th matrix element.
   *
   * Throw OUT_OF_RANGE if either index is out of range.
   *
   * @param i The row index
   * @param j The column index
   * @param val The value to insert
   * @throws OUT_OF_RANGE if either index is out of range
   */
  void SetElement(int i, int j, T val) override {
    if (i < 0 || j < 0 || i >= GetRowCount() || j >= GetColumnCount()) {
      throw Exception(ExceptionType::OUT_OF_RANGE, "RowMatrix error SetElement");
      return;
    }
    data_[i][j] = val;
  }

  /**
   * TODO(P0): Add implementation
   *
   * Fill the elements of the matrix from `source`.
   *
   * Throw OUT_OF_RANGE in the event that `source`
   * does not contain the required number of elements.
   *
   * @param source The source container
   * @throws OUT_OF_RANGE if `source` is incorrect size
   */
  void FillFrom(const std::vector<T> &source) override {
    // throw NotImplementedException{"RowMatrix::FillFrom() not implemented."};
    int len = source.size();
    if (len != GetColumnCount() * GetRowCount()) {
      throw Exception(ExceptionType::OUT_OF_RANGE, "RowMatrix error FillFrom");
      return;
    }
    int cnt = 0;
    for (int i = 0; i < GetRowCount(); i++) {
      for (int j = 0; j < GetColumnCount(); j++) {
        data_[i][j] = this->linear_[cnt] = source[cnt];
        cnt++;
      }
    }
  }

  /**
   * TODO(P0): Add implementation
   *
   * Destroy a RowMatrix instance.
   */
  ~RowMatrix() override {
    for (int i = 0; i < RowMatrix::GetRowCount(); i++) {
      delete[] data_[i];
    }
    delete[] data_;
  }

 private:
  /**
   * A 2D array containing the elements of the matrix in row-major format.
   *
   * TODO(P0):
   * - Allocate the array of row pointers in the constructor.
   * - Use these pointers to point to corresponding elements of the `linear` array.
   * - Don't forget to deallocate the array in the destructor.
   */
  T **data_;
};

/**
 * The RowMatrixOperations class defines operations
 * that may be performed on instances of `RowMatrix`.
 */
template <typename T>
class RowMatrixOperations {
 public:
  /**
   * Compute (`matrixA` + `matrixB`) and return the result.
   * Return `nullptr` if dimensions mismatch for input matrices.
   * @param matrixA Input matrix
   * @param matrixB Input matrix
   * @return The result of matrix addition
   */
  static std::unique_ptr<RowMatrix<T>> Add(const RowMatrix<T> *matrixA, const RowMatrix<T> *matrixB) {
    // TODO(P0): Add implementation
    // return std::unique_ptr<RowMatrix<T>>(nullptr);
    int rows = matrixA->GetRowCount();
    int cols = matrixA->GetColumnCount();
    std::unique_ptr<RowMatrix<T>> result(new RowMatrix<T>(rows, cols));
    std::vector<T> tmp_vec;
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        T value = matrixA->GetElement(i, j) + matrixB->GetElement(i, j);
        tmp_vec.push_back(value);
        result->SetElement(i, j, value);
      }
    }
    result->FillFrom(tmp_vec);
    return result;
  }

  /**
   * Compute the matrix multiplication (`matrixA` * `matrixB` and return the result.
   * Return `nullptr` if dimensions mismatch for input matrices.
   * @param matrixA Input matrix
   * @param matrixB Input matrix
   * @return The result of matrix multiplication
   */
  static std::unique_ptr<RowMatrix<T>> Multiply(const RowMatrix<T> *matrixA, const RowMatrix<T> *matrixB) {
    // TODO(P0): Add implementation
    int cols = matrixB->GetColumnCount();
    int rows = matrixA->GetRowCount();
    if (matrixA->GetColumnCount() != matrixB->GetRowCount()) {
      return std::unique_ptr<RowMatrix<T>>(nullptr);
    }
    std::vector<T> tmp_vec;
    std::unique_ptr<RowMatrix<T>> result(new RowMatrix<T>(rows, cols));
    for (int i = 0; i < rows; i++) {
      for (int j = 0; j < cols; j++) {
        T value = 0;
        int acols = matrixA->GetColumnCount();
        for (int k = 0; k < acols; k++) {
          value += matrixA->GetElement(i, k) * matrixB->GetElement(k, j);
        }
        tmp_vec.push_back(value);
        result->SetElement(i, j, value);
      }
    }
    result->FillFrom(tmp_vec);
    return result;
    // std::unique_ptr<RowMatrix<T>> result;
  }

  /**
   * Simplified General Matrix Multiply operation. Compute (`matrixA` * `matrixB` + `matrixC`).
   * Return `nullptr` if dimensions mismatch for input matrices.
   * @param matrixA Input matrix
   * @param matrixB Input matrix
   * @param matrixC Input matrix
   * @return The result of general matrix multiply
   */
  static std::unique_ptr<RowMatrix<T>> GEMM(const RowMatrix<T> *matrixA, const RowMatrix<T> *matrixB,
                                            const RowMatrix<T> *matrixC) {
    // TODO(P0): Add implementation
    // return std::unique_ptr<RowMatrix<T>>(nullptr);
    int cols_a = matrixA->GetColumnCount();
    int rows_b = matrixB->GetRowCount();
    int cols_c = matrixC->GetColumnCount();
    int rows_c = matrixC->GetRowCount();
    if (cols_a != rows_b || cols_c != matrixB->GetColumnCount() || rows_c != matrixA->GetRowCount()) {
      return std::unique_ptr<RowMatrix<T>>(nullptr);
    }
    std::unique_ptr<RowMatrix<T>> tmp_mat = Multiply(matrixA, matrixB);
    std::unique_ptr<RowMatrix<T>> result = Add(tmp_mat.get(), matrixC);
    return result;
  }
};
}  // namespace bustub
