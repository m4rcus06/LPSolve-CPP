# Linear Programming Solver

Giao diện đồ họa (GUI) hỗ trợ giải bài toán **Quy hoạch Tuyến tính tổng quát** (General Linear Programming) bằng **Phương pháp Simplex** hai pha, kết hợp sức mạnh của C++ và Python.

---

## Cấu trúc dự án

```
myProject/
├── LPSolver.bat          # File chạy CHÍNH - Double-click để khởi động
├── install.bat           # Script cài đặt và biên dịch
├── README.md             # Tài liệu hướng dẫn
├── gui/
│   ├── lp_gui.py        # Giao diện người dùng (Python/Tkinter)
│   └── requirements.txt  # Các thư viện Python cần thiết
├── src/
│   ├── LPInterface.cpp   # Wrapper giao tiếp Python-C++
│   └── include/          # Thư viện C++ xử lý Simplex
│       ├── matrix.h          # Template class ma trận
│       ├── linear_program.h  # Cấu trúc bài toán QHTT
│       └── LPSolver.h        # Thuật toán Simplex hai pha
└── testing/              # Các bài test đơn vị
```

---

## Tính năng chính

### 1. Nhập bài toán Quy hoạch Tuyến tính tổng quát
- Số biến: 1 - 20
- Số ràng buộc: 1 - 20
- Hệ số hàm mục tiêu và hệ số tự do
- Ma trận ràng buộc với các quan hệ: ≤, ≥, =
- Loại biến: ≥0, ≤0, Free (không giới hạn)
- Tối ưu hóa: Min / Max
- Luật chọn pivot: Dantzig / Bland

### 2. Giải bài toán
- Thuật toán Simplex hai pha (Two-Phase Simplex Method)
- Phát hiện tự động: nghiệm tối ưu (OPTIMAL), vô nghiệm (INFEASIBLE), không giới nội (UNBOUNDED)
- Hiển thị nghiệm tối ưu và giá trị hàm mục tiêu

### 3. Hiển thị Dictionary Form
- Xem chi tiết từng bước lặp của thuật toán
- Chế độ xem từng bước với nút điều hướng ◀ ▶

### 4. Trực quan hóa (bài toán 2 biến)
- Vẽ miền chấp nhận được
- Hiển thị đường đi qua các đỉnh
- Đánh dấu điểm bắt đầu, trung gian và điểm tối ưu

---

## Hướng dẫn cài đặt & sử dụng

### Yêu cầu hệ thống

- **Python 3.7+** (khuyến nghị Python 3.11 trở lên)
- **GCC compiler** (MinGW-w64 hoặc MSYS2)
- Hệ điều hành: Windows 10/11

### Các bước cài đặt

#### Bước 1: Chạy file cài đặt

Double-click vào file **`LPSolver.bat`** (hoặc `install.bat` nếu muốn chỉ cài đặt mà không mở GUI).

#### Bước 2: Đợi quá trình cài đặt hoàn tất

Script sẽ tự động thực hiện:
- Kiểm tra Python đã cài đặt chưa
- Cài đặt các thư viện Python cần thiết (`numpy`, `matplotlib`)
- Biên dịch C++ solver thành file thực thi

Nếu gặp lỗi, script sẽ hiển thị thông báo chi tiết.

---

### Hướng dẫn sử dụng chi tiết

#### 1. Giao diện chính

Sau khi khởi động thành công, giao diện GUI sẽ hiển thị với các thành phần:

```
┌─────────────────────────────────────────────────────────────┐
│  [Số biến: __] [Số ràng buộc: __] [Tạo bảng]              │
├─────────────────────────────────────────────────────────────┤
│  Mục tiêu:  ○ MAX    ○ MIN    Luật pivot: [Dantzig ▼]     │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Hàm mục tiêu:                                             │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Hệ số:  [  ] [  ] [  ]                             │   │
│  │  Hằng số: [  ]                                      │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  Ràng buộc:                                                │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  Hệ số         Quan hệ    RHS                        │   │
│  │  [  ][  ]  ≤   [  ]                                 │   │
│  │  [  ][  ]  ≤   [  ]                                 │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
│  Biến:                                                     │
│  ┌─────────────────────────────────────────────────────┐   │
│  │  x₁: [≥0 ▼]   x₂: [≥0 ▼]                           │   │
│  └─────────────────────────────────────────────────────┘   │
│                                                             │
├─────────────────────────────────────────────────────────────┤
│  [Solve]  [Dictionary]  [Clear]  [Example]                │
└─────────────────────────────────────────────────────────────┘
```

#### 2. Nhập số biến và ràng buộc

- **Số biến**: Nhập số lượng biến quyết định (1-20)
- **Số ràng buộc**: Nhập số lượng ràng buộc (1-20)
- Click **"Tạo bảng"** để sinh bảng nhập liệu

#### 3. Thiết lập bài toán

**a) Chọn mục tiêu:**
- **MAX**: Tối đa hóa hàm mục tiêu
- **MIN**: Tối thiểu hóa hàm mục tiêu

**b) Nhập hàm mục tiêu:**
- **Hệ số**: Nhập các hệ số của từng biến (ví dụ: 3, 2 cho `3x₁ + 2x₂`)
- **Hằng số**: Nhập hệ số tự do (nếu có, ví dụ: `Z = 3x₁ + 2x₂ + 5`)

**c) Nhập ràng buộc:**
- **Hệ số**: Nhập ma trận hệ số A
- **Quan hệ**: Chọn `≤`, `≥`, hoặc `=`
- **RHS**: Nhập vector b (vế phải)

**d) Thiết lập loại biến:**
- **≥0**: Biến không âm (mặc định)
- **≤0**: Biến không dương
- **Free**: Biến không giới hạn dấu

#### 4. Chọn luật pivot

- **Dantzig**: Chọn cột có hệ số âm lớn nhất (nhanh hơn, có thể bị cycling)
- **Bland**: Chọn cột có chỉ số nhỏ nhất (luôn hội tụ, chậm hơn)

#### 5. Giải bài toán

Click nút **"Solve"** để giải. Kết quả hiển thị:
- Trạng thái: `OPTIMAL` (tối ưu), `INFEASIBLE` (vô nghiệm), `UNBOUNDED` (không giới nội)
- Giá trị các biến
- Giá trị hàm mục tiêu tối ưu

#### 6. Xem Dictionary (các bước Simplex)

Click nút **"Dictionary"** để xem chi tiết từng bước lặp:

```
┌─────────────────────────────────────────────────────────────┐
│  Dictionary Step Viewer                           [X]       │
├─────────────────────────────────────────────────────────────┤
│  [◀] Step 3/5 [▶]  [Auto Play]                           │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  Z = -24 - 8*x1 - 6*w1                                    │
│                                                             │
│  x2 = 4 - 2*x1 - w1                                       │
│  w2 = 0 - x1 - w1                                         │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

- Nút **◀/▶**: Xem bước trước/sau
- **Auto Play**: Tự động chạy qua các bước

#### 7. Trực quan hóa (2 biến)

Với bài toán 2 biến, kết quả tự động hiển thị đồ thị:
- Miền chấp nhận được (vùng xanh)
- Các đường ràng buộc
- Đường đi qua các đỉnh
- Điểm tối ưu (đánh dấu màu đỏ)

#### 8. Các nút chức năng

| Nút | Chức năng |
|-----|-----------|
| **Solve** | Giải bài toán và hiển thị kết quả |
| **Dictionary** | Xem các bước lặp Simplex |
| **Clear** | Xóa tất cả dữ liệu đã nhập |
| **Example** | Tải bài toán mẫu có sẵn |

---

### Ví dụ minh họa

#### Ví dụ 1: Bài toán MAX (mẫu có sẵn)

Tải nhanh bằng nút **"Example"**:

```
Max Z = 3x₁ + 2x₂

s.t.    2x₁ + x₂ ≤ 4
        x₁ + 2x₂ ≤ 12
        x₁ + x₂ ≤ 8
        x₁, x₂ ≥ 0
```

**Cách nhập:**
1. Chọn: Số biến = 2, Số ràng buộc = 3 → "Tạo bảng"
2. Mục tiêu: **MAX**, Luật pivot: **Dantzig**
3. Hàm mục tiêu: Hệ số = `[3, 2]`, Hằng số = `0`
4. Ràng buộc 1: Hệ số = `[2, 1]`, Quan hệ = `≤`, RHS = `4`
5. Ràng buộc 2: Hệ số = `[1, 2]`, Quan hệ = `≤`, RHS = `12`
6. Ràng buộc 3: Hệ số = `[1, 1]`, Quan hệ = `≤`, RHS = `8`
7. Biến: `x₁ ≥ 0`, `x₂ ≥ 0`
8. Click **"Solve"**

**Kết quả:** x₁ = 0, x₂ = 4, Max Z = 8

---

#### Ví dụ 2: Bài toán MIN với ràng buộc ≥

```
Min Z = 4x₁ + 6x₂

s.t.    2x₁ + x₂ ≤ 4
        x₁ + x₂ ≥ 4
        x₁, x₂ ≥ 0
```

**Cách nhập:**
1. Chọn: Số biến = 2, Số ràng buộc = 2 → "Tạo bảng"
2. Mục tiêu: **MIN**
3. Hàm mục tiêu: Hệ số = `[4, 6]`
4. Ràng buộc 1: Hệ số = `[2, 1]`, Quan hệ = `≤`, RHS = `4`
5. Ràng buộc 2: Hệ số = `[1, 1]`, Quan hệ = `≥`, RHS = `4`
6. Biến: `x₁ ≥ 0`, `x₂ ≥ 0`
7. Click **"Solve"**

**Kết quả:** x₁ = 0, x₂ = 4, Min Z = 24

---

#### Ví dụ 3: Bài toán với biến Free

```
Max Z = x₁ + x₂

s.t.    x₁ + 2x₂ ≤ 8
        2x₁ + x₂ ≤ 8
        x₁, x₂ free (không giới hạn dấu)
```

**Cách nhập:**
1. Chọn: Số biến = 2, Số ràng buộc = 2 → "Tạo bảng"
2. Mục tiêu: **MAX**
3. Biến: `x₁ = Free`, `x₂ = Free` (chọn "Free" từ dropdown)

---

## Xử lý lỗi thường gặp

### "Python is not installed" hoặc "Python not found"

**Nguyên nhân:** Python chưa được cài đặt hoặc chưa được thêm vào PATH.

**Cách khắc phục:**
1. Tải Python từ: https://python.org
2. Khi cài đặt, **QUAN TRỌNG**: Tick chọn **"Add Python to PATH"**
3. Sau khi cài xong, tắt và mở lại terminal, chạy `python --version` để kiểm tra

### "GCC compiler not found" hoặc lỗi biên dịch C++

**Nguyên nhân:** Trình biên dịch C++ (GCC/MinGW) chưa được cài đặt.

**Cách khắc phục:**
1. Cài đặt **MinGW-w64** từ: https://www.mingw-w64.org/
2. Hoặc cài đặt **MSYS2**, sau đó chạy: `pacman -S mingw-w64-gcc`
3. Kiểm tra bằng lệnh: `g++ --version`

### "Solver produced no output" hoặc GUI không hiển thị kết quả

**Nguyên nhân:** File thực thi C++ chưa được biên dịch hoặc bị lỗi.

**Cách khắc phục:**
1. Double-click **`install.bat`** (không phải LPSolver.bat)
2. Đợi quá trình biên dịch hoàn tất
3. Nếu vẫn lỗi, kiểm tra trong thư mục `src/` đã có file `LPInterface.exe` hoặc `LPInterface` chưa

### Chương trình bị treo khi giải bài toán lớn

**Nguyên nhân:** Bài toán có nhiều biến/ràng buộc hoặc thuật toán bị vòng lặp (cycling).

**Cách khắc phục:**
1. Thử chọn **Luật Bland** thay vì Dantzig
2. Kiểm tra lại dữ liệu nhập vào
3. Giảm số biến/ràng buộc nếu có thể

### Đồ thị không hiển thị

**Nguyên nhân:** Chỉ hỗ trợ bài toán **2 biến**.

**Cách khắc phục:**
- Đảm bảo số biến = 2
- Kiểm tra cửa sổ đồ thị không bị minimize

### Lỗi "numpy not found" hoặc "matplotlib not found"

**Nguyên nhân:** Thư viện Python chưa được cài đặt.

**Cách khắc phục:**
1. Mở Command Prompt
2. Chạy lệnh:
   ```
   pip install numpy matplotlib
   ```

---

## Credits

| Thành phần | Công nghệ | Đóng góp |
|------------|-----------|-----------|
| **Core Solver** | C++17 | Thiết kế kiến trúc, giải thuật Simplex, xử lý ma trận, testing - *với sự hỗ trợ của Gemini AI* |
| **Graphical Interface** | Python / Tkinter | Phát triển GUI, trực quan hóa, tương tác người dùng - *bởi Cursor AI* |

---

## Giấy phép

MIT License
