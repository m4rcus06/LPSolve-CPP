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
- Số biến: 1 - 20 (Do giới hạn UI)
- Số ràng buộc: 1 - 20 (Do giới hạn UI)
- Hệ số hàm mục tiêu và hệ số tự do
- Ma trận ràng buộc với các quan hệ: ≤, ≥, =
- Loại biến: ≥0, ≤0, Free (không giới hạn)
- Tối ưu hóa: Min / Max
- Luật chọn pivot: Dantzig / Bland

### 2. Giải bài toán
- Thuật toán Simplex hai pha (Two-Phase Simplex Method)
- Phát hiện tự động: nghiệm tối ưu (OPTIMAL), vô nghiệm (INFEASIBLE), không giới nội (UNBOUNDED), rơi vào vòng lặp (CYCLING)
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

### Các bước thực hiện

1. **Double-click** `LPSolver.bat` để khởi động
2. Lần đầu chạy, script sẽ tự động:
   - Kiểm tra và cài đặt Python (nếu chưa có)
   - Cài đặt các thư viện: `numpy`, `matplotlib`
   - Biên dịch C++ solver
3. Giao diện GUI sẽ tự động mở ra

---

## Ví dụ minh họa

Bài toán mẫu (tải nhanh bằng nút **"Example"**):

```
Max Z = 3x₁ + 2x₂

s.t.    2x₁ + x₂ ≤ 4
        x₁ + 2x₂ ≤ 12
        x₁ + x₂ ≤ 8
        x₁, x₂ ≥ 0
```

**Kết quả:** x₁ = 0, x₂ = 4, Max Z = 8

---

## Xử lý lỗi thường gặp

### "Python is not installed"
- Tải Python từ: https://python.org
- Khi cài đặt, đảm bảo chọn **"Add Python to PATH"**

### "GCC compiler not found"
- Cài đặt MinGW-w64: https://www.mingw-w64.org/
- Hoặc sử dụng MSYS2: `pacman -S mingw-w64-gcc`

### "Solver produced no output"
- Chạy lại `install.bat` để biên dịch lại C++ solver

---

## Credits

| Thành phần | Công nghệ | Đóng góp |
|------------|-----------|-----------|
| **Core Solver** | C++17 | Thiết kế kiến trúc, giải thuật Simplex, xử lý ma trận, testing - *Do cá nhân lên ý tưởng, với sự hỗ trợ của Gemini AI trong tác vụ debug, kiểm tra logic* |
| **Graphical Interface** | Python / Tkinter | Phát triển GUI, trực quan hóa, tương tác người dùng - *bởi Cursor AI* |

---

## Giấy phép

MIT License
