#ifndef CSV_PARSER_H__
#define CSV_PARSER_H__


/*****************************************************************************/
/*                                                                           */
/*   CSV�t�@�C���̓��o�͂���уf�[�^�ǂݏ������s�����߂̃N���X�̒�`�E����   */
/*                                                                           */
/*****************************************************************************/

// ���l�F
//   - ����ł� UTF-8, EUC-JP, Shift_JIS �݂̂�z�肵�Ă���C����ȊO�ɂ͖��Ή�
//   - �}���`�o�C�g�����𐳏�Ƀp�[�X�ł��Ȃ��ꍇ����


#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "Noncopyable.h"


// namespace kn: Kazuaki Nakamura �̗�
namespace kn
{
	// CSVParser�N���X�Ŏg�p����G���[�R�[�h�^
	enum CSV_PARSER_ERROR_CODE
	{
		CSV_PARSER_OK, // ����I��
		CSV_PARSER_INVALID_PARAMETER, // ���\�b�h�̈������s��
		CSV_PARSER_INVALID_ISTREAM, // �����ȓ��̓X�g���[��
		CSV_PARSER_INVALID_OSTREAM, // �����ȏo�̓X�g���[��
		CSV_PARSER_OUT_OF_ARRAY // �z��O�A�N�Z�X
	};

	// CSVParser�N���X�őΏۂƂ��镶���G���R�[�f�B���O���
	enum CSV_PARSER_ENCODING
	{
		CSV_PARSER_UTF_8,
		CSV_PARSER_EUC_JP,
		CSV_PARSER_SHIFT_JIS
	};

	// CSV�t�@�C���p�[�X�p�N���X�̒�`�E����
	class CSVParser : private Noncopyable
	{
	private:
		// ���䕶���ݒ�
		int m_Quoter;    // ���p���i�f�t�H���g�ł̓_�u���N�H�[�g�j
		int m_Delimiter; // ��؂蕶���i�f�t�H���g�ł̓J���}�j

		// CSV�f�[�^�̎��̂��i�[����񎟌��z��
		std::vector<std::vector<std::string>> m_Data;

		// �񎟌��z��̍s���E��
		unsigned int m_nRows; // �s��
		unsigned int m_nCols; // �񐔁i�̍ő�l�j

		// �z��O�A�N�Z�X���p�̃_�~�[������
		const std::string m_NullString;

	private:
		// �P��f�[�^�o�͊֐�
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

		// �}���`�o�C�g���������p�֐��iUTF-8�p�j
		static inline void mbchar_parse_utf8(int& c, int c_dmy, std::string& ch, std::istream& ist)
		{
			if (c < 0xC0) { // 1�o�C�g����
				/* �������Ȃ�*/ ;
			}
			else if (c <= 0xDF) { // 2�o�C�g����
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xEF) { // 3�o�C�g����
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xF7) { // 4�o�C�g����
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xFB) { // 5�o�C�g����
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (c <= 0xFD) { // 6�o�C�g����
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
		}

		// �}���`�o�C�g���������p�֐��iEUC-JP�p�j
		static inline void mbchar_parse_eucjp(int& c, int c_dmy, std::string& ch, std::istream& ist)
		{
			if (c == 0x8F) { // 3�o�C�g����
				ch += static_cast<char>(ist.get());
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
			else if (0x80 <= c && c <= 0xFF) { // 2�o�C�g����
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
		}

		// �}���`�o�C�g���������p�֐��iShift-JIS�p�j
		static inline void mbchar_parse_shiftjis(int& c, int c_dmy, std::string& ch, std::istream& ist)
		{
			if ((0x81 <= c && c <= 0x9f) || (0xE0 <= c && c <= 0xFC)) { // 2�o�C�g����
				ch += static_cast<char>(ist.get());
				c = c_dmy;
			}
		}

	public:
		// �R���X�g���N�^
		CSVParser()
			: m_Quoter('"'),
			  m_Delimiter(','),
			  m_NullString(""),
			  m_nRows(0),
			  m_nCols(0)
		{
			// ���o�[�W�����ł͉������Ȃ�
		}
		CSVParser(int param_Quoter, int param_Delimiter)
			: m_Quoter(param_Quoter),
			  m_Delimiter(param_Delimiter),
			  m_NullString(""),
			  m_nRows(0),
			  m_nCols(0)
		{
			// ���o�[�W�����ł͉������Ȃ�
		}

		// ���݂̈��p���E��؂蕶���ݒ���m�F����
		inline int GetQuoter() const
		{
			return m_Quoter;
		}
		inline int GetDelimiter() const
		{
			return m_Delimiter;
		}

		// ���p���E��؂蕶���ݒ��ύX����
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

		// ���݂�CSV�f�[�^���N���A����
		inline CSV_PARSER_ERROR_CODE ClearData()
		{
			m_Data.clear();
			m_nRows = 0;
			m_nCols = 0;
			return CSV_PARSER_OK;
		}

		// ���݂�CSV�f�[�^���N���A���C�X�Ɉ��p���E��؂蕶���ݒ���f�t�H���g�ɖ߂�
		inline CSV_PARSER_ERROR_CODE Reset()
		{
			m_Data.clear();
			m_nRows = 0;
			m_nCols = 0;
			m_Quoter = '"';
			m_Delimiter = ',';
			return CSV_PARSER_OK;
		}

		// rows �s�@cols ���CSV�f�[�^��V�K�쐬���C�e�t�B�[���h�� data �ŏ���������
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

		// r �s c ��̃f�[�^�ɃA�N�Z�X
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
		inline int GetDataAsInt(unsigned int r, unsigned int c) const // r �s c ��̃f�[�^�� int �l�Ƃ��ēǂݍ���
		{
			if (r >= m_Data.size()) return 0;
			else if (c >= m_Data[r].size()) return 0;
			else return atoi(m_Data[r][c].c_str());
		}
		inline double GetDataAsDouble(unsigned int r, unsigned int c) const // r �s c ��̃f�[�^�� double �l�Ƃ��ēǂݍ���
		{
			if (r >= m_Data.size()) return 0.0;
			else if (c >= m_Data[r].size()) return 0.0;
			else return atof(m_Data[r][c].c_str());
		}
		inline long long GetDataAsLLong(unsigned int r, unsigned int c) const // r �s c ��̃f�[�^�� long long �l�Ƃ��ēǂݍ���
		{
			if (r >= m_Data.size()) return 0ll;
			else if (c >= m_Data[r].size()) return 0ll;
			else return atoll(m_Data[r][c].c_str());
		}

		// �s���E�񐔂̎擾
		inline unsigned int RowSize() const // �s��
		{
			return m_nRows;
		}
		inline unsigned int ColSize() const // �񐔁i�̍ő�l�j
		{
			return m_nCols;
		}
		inline unsigned int ColSize(unsigned int r) const // r �s�ڂ̎�����
		{
			if (r >= m_Data.size()) return 0;
			else return static_cast<unsigned int>(m_Data[r].size());
		}

		// r �s c ��Ƀf�[�^���Z�b�g
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

		// r �s c ��ɐ��l�f�[�^�𕶎���Ƃ��ăZ�b�g
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

		// r �s�ڂ̒��O�ɍs�f�[�^ rowData ��}������ir < m_nRows �̎��̂ݗL���j
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

		// ���݂̃f�[�^�z��̖����ɍs�f�[�^ rowData ��}������
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

		// r �s�ڂ̃f�[�^���폜����ir < m_nRows �̎��̂ݗL���j
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

		// c �s�ڂ̒��O�ɗ�f�[�^ colData ��}������ic < m_nCols �̎��̂ݗL���j
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

		// ���݂̃f�[�^�z��̖����ɗ�f�[�^ colData ��}������
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

		// c ��ڂ̃f�[�^���폜����ic < m_nCols �̎��̂ݗL���j
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

		// CSV�t�@�C������f�[�^��ǂݍ���
		// �f�t�H���g�ł͕����R�[�h�Ƃ��� Shift-JIS �����肳���
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

			// ���݂̃f�[�^���N���A
			m_Data.clear();

			// ���̓X�g���[���̃`�F�b�N
			if (!ist.good()) return CSV_PARSER_INVALID_ISTREAM;

			// �}���`�o�C�g�����񏈗����̃_�~�[�����̐ݒ�
			const int c_dmy = (m_Delimiter == 'a') ? (((m_Quoter == 'b') ? 'c' : 'b')) : (((m_Quoter == 'a') ? ((m_Delimiter == 'b') ? 'c' : 'b') : 'a'));

			// �}���`�o�C�g���������p�֐��̐ݒ�
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

			// CSV�t�@�C���p�[�X
			while (1) {
				// �t�@�C������1�����ǂݍ��ށD�t�@�C���I�[�ɒB������I���D
				if ((c = ist.get()) == EOF) {
					if (!data.empty()) line.push_back(data);
					if (!line.empty()) m_Data.push_back(line);
					break;
				}
				ch = static_cast<char>(c);

				// �}���`�o�C�g�����ɑΏ�
				(*mbchar_parse)(c, c_dmy, ch, ist);

				// �ǂݍ��񂾕����ɉ������������s��
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

		// �f�[�^���t�@�C���ɏ����o��
		// �f�t�H���g�ł͏㏑�����[�h�ɂȂ邪�Capp_mode �� true ���Z�b�g����ƒǋL���[�h�ɂȂ�
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

		// �f�[�^���X�g���[���ɏ����o��
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
