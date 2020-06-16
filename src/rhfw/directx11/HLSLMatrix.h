/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef HLSLMATRIX_H_
#define HLSLMATRIX_H_

namespace rhfw {

template<int RowLength, int ColumnLength>
class HLSLMatrix {
public:
	float data[(ColumnLength - 1) * 4 + RowLength];

	HLSLMatrix() {
	}

	template<int Dim>
	HLSLMatrix(const Matrix<Dim>& m) {
		//TODO support non nxn matrices
		static_assert(Dim == RowLength && Dim == ColumnLength, "Invalid matrix conversion");
		for (int i = 0; i < ColumnLength; ++i) {
			for (int j = 0; j < RowLength; ++j) {
				this->data[i * 4 + j] = m[i * Dim + j];
			}
		}
	}
};

} // namespace rhfw

#endif /* HLSLMATRIX_H_ */
