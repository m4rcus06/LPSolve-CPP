# Linear Programming Solver - GUI

Giao diện đồ họa để giải bài toán Quy hoạch tuyến tính tổng quát bằng phương pháp Simplex.

## Cấu trúc dự án

```
myProject/
├── run.bat              # File chạy CHÍNH - Double-click để chạy!
├── README.md            # Hướng dẫn sử dụng
├── gui/
│   ├── lp_gui.py       # Giao diện Python GUI
│   └── requirements.txt # Python dependencies
├── src/
│   ├── LPInterface.cpp  # Wrapper C++
│   └── include/         # Thư viện C++
│       ├── matrix.h
│       ├── linear_program.h
│       └── LPSolver.h
└── testing/            # Unit tests
```

## Cách sử dụng

1. Double-click **`run.bat`**
2. Script sẽ tự động:
   - Cài đặt Python packages (numpy, matplotlib)
   - Compile C++ solver
   - Mở giao diện GUI

## Tính năng

1. **Nhập bài toán QHTT tổng quát:**
   - Chọn số biến (1-20) và số ràng buộc (1-20)
   - Nhập hệ số hàm mục tiêu
   - Nhập ma trận ràng buộc với các dấu (≤, ≥, =)
   - Chọn dấu của biến (≥0, ≤0, Free)
   - Chọn Min/Max cho hàm mục tiêu
   - Chọn Pivot Rule (Dantzig/Bland)

2. **Giải bài toán:**
   - Gọi solver C++ từ Python
   - Hiển thị nghiệm tối ưu và giá trị hàm mục tiêu

3. **Hiển thị Dictionary Form:**
   - Xem dạng từ điển của bài toán qua mỗi iteration

4. **Trực quan hóa (cho bài toán 2 biến):**
   - Vẽ miền chấp nhận được
   - Hiển thị đường đi của biến qua các iteration
   - Đánh dấu điểm bắt đầu, trung gian và điểm tối ưu

## Ví dụ

Bài toán mẫu (tải bằng nút "Example"):

```
Max Z = 3x₁ + 2x₂

s.t. 2x₁ + x₂ ≤ 4
     x₁ + 2x₂ ≤ 12
     x₁ + x₂ ≤ 8
     x₁, x₂ ≥ 0
```

**Kết quả:** x₁ = 0, x₂ = 4, Max Z = 8

## Yêu cầu hệ thống

- Python 3.7+
- NumPy
- Matplotlib
- GCC compiler (MinGW-w64)

## Troubleshooting

### Lỗi "Python is not installed"
- Tải Python từ: https://python.org
- Đảm bảo tick chọn "Add Python to PATH"

### Lỗi "GCC compiler not found"
- Cài đặt MinGW-w64: https://mingw-w64.org/
- Hoặc dùng MSYS2: `pacman -S mingw-w64-gcc`
