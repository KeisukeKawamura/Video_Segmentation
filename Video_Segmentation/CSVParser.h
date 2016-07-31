#ifndef CSV_PARSER_H__
#define CSV_PARSER_H__


/*****************************************************************************/
/*                                                                           */
/*   CSVファイルの入出力およびデータ読み書きを行うためのクラスの定義・実装   */
/*                                                                           */
/*****************************************************************************/

// 備考：
//   - 現状では UTF-8, EUC-JP, Shift_JIS のみを想定しており，それ以外には未対応
//   - マルチバイト文字を正常にパースできない場合あり


#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "Noncopyable.h"


// namespace kn: Kazuaki Nakamura の略
namespace kn
{
	// CSVParserクラスで使用するエラーコード型
	enum CSV_PARSER_ERROR_CODE
	{
		CSV_PARSER_OK, // 正常終了
		CSV_PARSER_INVALID_PARAMETER, // メソッドの引数が不正
		CSV_PARSER_INVALID_ISTREAM, // 無効な入力ストリーム
		CSV_PARSER_INVALID_OSTREAM, // 無効な出力ストリーム
		CSV_PARSER_OUT_OF_ARRAY // 配列外アクセス
	};

	// CSVParserクラスで対象とする文字エンコーディング種別
	enum CSV_PARSER_ENCODING
	{
		CSV_PARSER_UTF_8,
		CSV_PARSER_EUC_JP,
		CSV_PARSER_SHIFT_JIS
	};

	// CSVファイルパース用クラスの定義・実装
	class CSVParser : private Noncopyable
	{
	private:
		// 制御文字設定
		int m_Quoter;    // 引用符（デフォルトではダブルクォート）
		int m_Delimiter; // 区切り文字（デフォルトではカンマ）

		// CSVデータの実体を格納する二次元配列
		std::vector<std::vector<std::string>> m_Data;

		// 二次元配列の行数・列数
		unsigned int m_nRows; // 行数
		unsigned int m_nCols; // 列数（の最大値）

		// 配列外アクセス時用のダミー文字列
		const std::string m_NullString;

	private:
		// 単一データ出力関数
		inline void print_data(unsigned int r, unsigned int c, std::ostream& ost) const
		{
			std::string::size_type b = 0;
			std::string::size_type e = 0;
			const bool printQuoteFlag = ( m_Data[r][c].find(m_Delimiter) == std::string::npos &&
										  m_Data[r][c].find('\n') == std::string::npos		  &&
										  m_Data[r][c].find('\r') == std::string::npos			 ) ? false : true;
			if (printQuoteFlag) ost << static_cast<char>(m_Quoter);
			while (1) {
				if ((e = m_Data[r][c].find(m_Quoter, b)) == std::string::npos) {
					ost << m_Data[r][c].substr(b);
					break;
				}
				ost << m_Data[r][c].substr(b, e - b + 1) << static_cast<char>(m_Quoter);
				b = e + 1;
			}
			if (printQuoteFlag) ost << static_cast<char>(m_Quoter);
		}

		// マルチバイト文字処理用関数（UTF-8用）
		static inline void mbchar_parse_utf8(int& c, int c_dmy, std::string& ch, std::istream& ist)
		{
			if (c < 0xC0) { // 1バイト文字
				/* 何もしない*/ ;
			}
			else if (c <= 0xDF) { // 2バイト文字
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xEF) { // 3バイト文字
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xF7) { // 4バイト文字
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xFB) { // 5バイト文字
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xFD) { // 6バイト文字
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
		}

		// マルチバイト文字処理用関数（EUC-JP用）
		static inline void mbchar_parse_eucjp(int& c, int c_dmy, std::string& ch, std::istream& ist)
		{
			if (c == 0x8F) { // 3バイト文字
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (0x80 <= c && c <= 0xFF) { // 2バイト文字
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
		}

		// マルチバイト文字処理用関数（Shift-JIS用）
		static inline void mbchar_parse_shiftjis(int& c, int c_dmy, std::string& ch, std::istream& ist)
		{
			if ((0x81 <= c && c <= 0x9f) || (0xE0 <= c && c <= 0xFC)) { // 2バイト文字
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
		}

	public:
		// コンストラクタ
		CSVParser()
			: m_Quoter('"'),
			  m_Delimiter(','),
			  m_NullString(""),
			  m_nRows(0),
			  m_nCols(0)
		{
			// 現バージョンでは何もしない
		}
		CSVParser(int param_Quoter, int param_Delimiter)
			: m_Quoter(param_Quoter),
			  m_Delimiter(param_Delimiter),
			  m_NullString(""),
			  m_nRows(0),
			  m_nCols(0)
		{
			// 現バージョンでは何もしない
		}

		// 現在の引用符・区切り文字設定を確認する
		inline int GetQuoter() const
		{
			return m_Quoter;
		}
		inline int GetDelimiter() const
		{
			return m_Delimiter;
		}

		// 引用符・区切り文字設定を変更する
		inline CSV_PARSER_ERROR_CODE SetQuoter(int param_Quoter)
		{
			m_Quoter = param_Quoter;
			return CSV_PARSER_OK;
		}
		inline CSV_PARSER_ERROR_CODE SetDelimiter(int param_Delimiter)
		{
			m_Delimiter = param_Delimiter;
			return CSV_PARSER_OK;
		}

		// 現在のCSVデータをクリアする
		inline CSV_PARSER_ERROR_CODE ClearData()
		{
			m_Data.clear();
			m_nRows = 0;
			m_nCols = 0;
			return CSV_PARSER_OK;
		}

		// 現在のCSVデータをクリアし，更に引用符・区切り文字設定をデフォルトに戻す
		inline CSV_PARSER_ERROR_CODE Reset()
		{
			m_Data.clear();
			m_nRows = 0;
			m_nCols = 0;
			m_Quoter = '"';
			m_Delimiter = ',';
			return CSV_PARSER_OK;
		}

		// rows 行　cols 列のCSVデータを新規作成し，各フィールドを data で初期化する
		inline CSV_PARSER_ERROR_CODE CreateData(unsigned int rows, unsigned int cols, const std::string& data)
		{
			if (rows == 0 || cols == 0) return CSV_PARSER_INVALID_PARAMETER;

			std::vector<std::string> dmy;
			m_Data.clear();
			m_nRows = rows;
			m_nCols = cols;
			dmy.assign(cols, data);
			m_Data.assign(rows, dmy);
			return CSV_PARSER_OK;
		}

		// r 行 c 列のデータにアクセス
		inline const std::string& operator()(unsigned int r, unsigned int c) const
		{
			if (r >= m_Data.size()) return m_NullString;
			else if (c >= m_Data[r].size()) return m_NullString;
			else return m_Data[r][c];
		}
		inline const std::string& GetData(unsigned int r, unsigned int c) const
		{
			if (r >= m_Data.size()) return m_NullString;
			else if (c >= m_Data[r].size()) return m_NullString;
			else return m_Data[r][c];
		}
		inline int GetDataAsInt(unsigned int r, unsigned int c) const // r 行 c 列のデータを int 値として読み込む
		{
			if (r >= m_Data.size()) return 0;
			else if (c >= m_Data[r].size()) return 0;
			else return atoi(m_Data[r][c].c_str());
		}
		inline double GetDataAsDouble(unsigned int r, unsigned int c) const // r 行 c 列のデータを double 値として読み込む
		{
			if (r >= m_Data.size()) return 0.0;
			else if (c >= m_Data[r].size()) return 0.0;
			else return atof(m_Data[r][c].c_str());
		}
		inline long long GetDataAsLLong(unsigned int r, unsigned int c) const // r 行 c 列のデータを long long 値として読み込む
		{
			if (r >= m_Data.size()) return 0ll;
			else if (c >= m_Data[r].size()) return 0ll;
			else return atoll(m_Data[r][c].c_str());
		}

		// 行数・列数の取得
		inline unsigned int RowSize() const // 行数
		{
			return m_nRows;
		}
		inline unsigned int ColSize() const // 列数（の最大値）
		{
			return m_nCols;
		}
		inline unsigned int ColSize(unsigned int r) const // r 行目の実質列数
		{
			if (r >= m_Data.size()) return 0;
			else return static_cast<unsigned int>(m_Data[r].size());
		}

		// r 行 c 列にデータをセット
		inline CSV_PARSER_ERROR_CODE SetData(unsigned int r, unsigned int c, const std::string& data)
		{
			if (r >= m_Data.size()) return CSV_PARSER_OUT_OF_ARRAY;
			else if (c >= m_nCols) return CSV_PARSER_OUT_OF_ARRAY;
			else {
				for (unsigned int k = static_cast<unsigned int>(m_Data[r].size()); k <= c; ++k) {
					m_Data[r].push_back("");
				}
				m_Data[r][c] = data;
				return CSV_PARSER_OK;
			}
		}

		// r 行 c 列に数値データを文字列としてセット
		template <typename TYPE>
		inline CSV_PARSER_ERROR_CODE SetDataAsNumber(unsigned int r, unsigned int c, TYPE val)
		{
			if (r >= m_Data.size()) return CSV_PARSER_OUT_OF_ARRAY;
			else if (c >= m_nCols) return CSV_PARSER_OUT_OF_ARRAY;
			else {
				std::stringstream ss;
				for (unsigned int k = static_cast<unsigned int>(m_Data[r].size()); k <= c; ++k) {
					m_Data[r].push_back("");
				}
				ss << val;
				m_Data[r][c] = ss.str();
				return CSV_PARSER_OK;
			}
		}

		// r 行目の直前に行データ rowData を挿入する（r < m_nRows の時のみ有効）
		CSV_PARSER_ERROR_CODE InsertRow(unsigned int r, std::vector<std::string>& rowData)
		{
			if (r >= m_nRows) return CSV_PARSER_INVALID_PARAMETER;
			if (rowData.empty()) return CSV_PARSER_INVALID_PARAMETER;

			std::vector<std::vector<std::string>>::iterator it = m_Data.begin();
			for (unsigned int k = 0; k < r; ++k) {
				++it;
			}
			it = m_Data.insert(it, rowData);
			if (m_nCols < rowData.size()) {
				m_nCols = static_cast<unsigned int>(rowData.size());
			}
			++m_nRows;
			return CSV_PARSER_OK;
		}

		// 現在のデータ配列の末尾に行データ rowData を挿入する
		CSV_PARSER_ERROR_CODE PushBackRow(std::vector<std::string>& rowData)
		{
			if (rowData.empty()) return CSV_PARSER_INVALID_PARAMETER;

			m_Data.push_back(rowData);
			if (m_nCols < rowData.size()) {
				m_nCols = static_cast<unsigned int>(rowData.size());
			}
			++m_nRows;
			return CSV_PARSER_OK;
		}

		// r 行目のデータを削除する（r < m_nRows の時のみ有効）
		CSV_PARSER_ERROR_CODE EraseRow(unsigned int r)
		{
			if (r >= m_nRows) return CSV_PARSER_INVALID_PARAMETER;

			std::vector<std::vector<std::string>>::iterator it = m_Data.begin();
			for (unsigned int k = 0; k < r; ++k) {
				++it;
			}
			it = m_Data.erase(it);
			m_nCols = 0;
			for (unsigned int k = 0; k < m_Data.size(); ++k) {
				if (m_nCols < m_Data.size()) m_nCols = static_cast<unsigned int>(m_Data.size());
			}
			--m_nRows;
			return CSV_PARSER_OK;
		}

		// c 行目の直前に列データ colData を挿入する（c < m_nCols の時のみ有効）
		CSV_PARSER_ERROR_CODE InsertCol(unsigned int c, std::vector<std::string>& colData)
		{
			if (c >= m_nCols) return CSV_PARSER_INVALID_PARAMETER;
			if (colData.empty()) return CSV_PARSER_INVALID_PARAMETER;

			for (unsigned int r = 0; r < m_Data.size(); ++r) {
				if (c < m_Data[r].size()) {
					std::vector<std::string>::iterator it = m_Data[r].begin();
					for (unsigned int k = 0; k < c; ++k) {
						++it;
					}
					if (r < colData.size()) it = m_Data[r].insert(it, colData[r]);
					else it = m_Data[r].insert(it, "");
				}
				else {
					for (unsigned int k = static_cast<unsigned int>(m_Data[r].size()); k < c; ++k) {
						m_Data[r].push_back("");
					}
					if (r < colData.size()) m_Data[r].push_back(colData[r]);
					else m_Data[r].push_back("");
				}
			}
			++m_nCols;
			return CSV_PARSER_OK;
		}

		// 現在のデータ配列の末尾に列データ colData を挿入する
		CSV_PARSER_ERROR_CODE PushBackCol(std::vector<std::string>& colData)
		{
			if (colData.empty()) return CSV_PARSER_INVALID_PARAMETER;

			for (unsigned int r = 0; r < m_Data.size(); ++r) {
				if (r >= colData.size()) break;
				for (unsigned int k = static_cast<unsigned int>(m_Data[r].size()); k < m_nCols; ++k) {
					m_Data[r].push_back("");
				}
				m_Data[r].push_back(colData[r]);
			}
			++m_nCols;
			return CSV_PARSER_OK;
		}

		// c 列目のデータを削除する（c < m_nCols の時のみ有効）
		CSV_PARSER_ERROR_CODE EraseCol(unsigned int c)
		{
			if (c >= m_nCols) return CSV_PARSER_INVALID_PARAMETER;

			for (unsigned int r = 0; r < m_Data.size(); ++r) {
				if (m_Data[r].empty()) continue;
				std::vector<std::string>::iterator it = m_Data[r].begin();
				for (unsigned int k = 0; k < c; ++k) {
					if (it == m_Data[r].end()) break;
					else ++it;
				}
				if (it != m_Data[r].end()) it = m_Data[r].erase(it);
			}
			--m_nCols;
			return CSV_PARSER_OK;
		}

		// CSVファイルからデータを読み込む
		// デフォルトでは文字コードとして Shift-JIS が仮定される
		CSV_PARSER_ERROR_CODE Parse(std::string filename, CSV_PARSER_ENCODING encoding = CSV_PARSER_SHIFT_JIS)
		{
			std::ifstream ifs(filename, std::ios::in);
			if (!ifs.is_open()) {
				m_Data.clear();
				return CSV_PARSER_INVALID_ISTREAM;
			}
			else {
				CSV_PARSER_ERROR_CODE code = this->Parse(ifs, encoding);
				ifs.close();
				return code;
			}
		}
		CSV_PARSER_ERROR_CODE Parse(std::istream& ist, CSV_PARSER_ENCODING encoding = CSV_PARSER_SHIFT_JIS)
		{
			int c = 0;
			std::string ch = "";
			std::string data = "";
			std::vector<std::string> line;
			enum state { S0, S1, S2, S3, S4 } s = S0;

			// 現在のデータをクリア
			m_Data.clear();

			// 入力ストリームのチェック
			if (!ist.good()) return CSV_PARSER_INVALID_ISTREAM;

			// マルチバイト文字列処理時のダミー文字の設定
			const int c_dmy = (m_Delimiter == 'a') ? (((m_Quoter == 'b') ? 'c' : 'b')) : (((m_Quoter == 'a') ? ((m_Delimiter == 'b') ? 'c' : 'b') : 'a'));

			// マルチバイト文字処理用関数の設定
			void(*mbchar_parse)(int&, int, std::string&, std::istream&) = NULL;
			switch (encoding) {
			case CSV_PARSER_UTF_8:
				mbchar_parse = kn::CSVParser::mbchar_parse_utf8;
				break;
			case CSV_PARSER_EUC_JP:
				mbchar_parse = kn::CSVParser::mbchar_parse_eucjp;
				break;
			case CSV_PARSER_SHIFT_JIS:
				mbchar_parse = kn::CSVParser::mbchar_parse_shiftjis;
			}

			// CSVファイルパース
			while (1) {
				// ファイルから1文字読み込む．ファイル終端に達したら終了．
				if ((c = ist.get()) == EOF) {
					if (!data.empty()) line.push_back(data);
					if (!line.empty()) m_Data.push_back(line);
					break;
				}
				ch = static_cast<char>(c);

				// マルチバイト文字に対処
				(*mbchar_parse)(c, c_dmy, ch, ist);

				// 読み込んだ文字に応じた処理を行う
				switch (s) {
				case S0:
					if (c == m_Delimiter) {
						line.push_back(data); data = "";
					}
					else if (c == '\n') {
						line.push_back(data); data = "";
						m_Data.push_back(line); line.clear();
					}
					else if (c == '\r') {
						line.push_back(data); data = "";
						m_Data.push_back(line); line.clear();
						s = S4;
					}
					else if (c == m_Quoter) s = S1;
					else data += ch;
					break;
				case S1:
					if (c == m_Quoter) {
						data += ch;
						s = S0;
					}
					else {
						data += ch;
						s = S2;
					}
					break;
				case S2:
					if (c == m_Quoter) s = S3;
					else data += ch;
					break;
				case S3:
					if (c == m_Delimiter) {
						line.push_back(data); data = "";
						s = S0;
					}
					else if (c == '\n') {
						line.push_back(data); data = "";
						m_Data.push_back(line); line.clear();
						s = S0;
					}
					else if (c == '\r') {
						line.push_back(data); data = "";
						m_Data.push_back(line); line.clear();
						s = S4;
					}
					else if (c == m_Quoter) {
						data += ch;
						s = S2;
					}
					else {
						data += ch;
						s = S0;
					}
					break;
				case S4:
					if (c == m_Delimiter) {
						line.push_back(data); data = "";
						s = S0;
					}
					else if (c == '\n') s = S0;
					else if (c == '\r') {
						line.push_back(data); data = "";
						m_Data.push_back(line); line.clear();
					}
					else if (c == m_Quoter) s = S1;
					else {
						data += ch;
						s = S0;
					}
				}
			}

			m_nCols = 0;
			m_nRows = static_cast<unsigned int>(m_Data.size());
			for (unsigned int k = 0; k < m_Data.size(); ++k) {
				if (m_nCols < m_Data[k].size()) m_nCols = static_cast<unsigned int>(m_Data[k].size());
			}

			return CSV_PARSER_OK;
		}

		// データをファイルに書き出す
		// デフォルトでは上書きモードになるが，app_mode に true をセットすると追記モードになる
		CSV_PARSER_ERROR_CODE Save(std::string filename, bool app_mode = false) const
		{
			std::ofstream ofs;
			if (app_mode) {
				ofs.open(filename, std::ios::app);
			}
			else {
				ofs.open(filename, std::ios::out);
			}
			if (!ofs.is_open()) {
				return CSV_PARSER_INVALID_OSTREAM;
			}
			CSV_PARSER_ERROR_CODE code = this->Print(ofs);
			ofs.close();
			return code;
		}

		// データをストリームに書き出す
		CSV_PARSER_ERROR_CODE Print(std::ostream& ost) const
		{
			if (!ost.good()) {
				return CSV_PARSER_INVALID_OSTREAM;
			}
			for (unsigned int j = 0; j < m_Data.size(); ++j) {
				if (m_Data[j].empty()) continue;
				this->print_data(j, 0, ost);
				for (unsigned int i = 1; i < m_Data[j].size(); ++i) {
					ost << ",";
					this->print_data(j, i, ost);
				}
				ost << std::endl;
			}
			return CSV_PARSER_OK;
		}
	};
}


#endif
