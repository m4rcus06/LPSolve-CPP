"""
Linear Programming Solver GUI
A graphical interface for solving General Linear Programming problems using Simplex Method.
"""

import tkinter as tk
from tkinter import ttk, messagebox, scrolledtext
import subprocess
import os
import re
import matplotlib
matplotlib.use('TkAgg')
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import numpy as np
from typing import Optional, Dict


class DictionaryStepViewer:
    """Step-by-step dictionary viewer window"""
    def __init__(self, parent, dictionaries):
        self.window = tk.Toplevel(parent)
        self.window.title("Dictionary Step Viewer")
        self.window.geometry("700x500")
        
        self.dictionaries = dictionaries
        self.current_index = 0
        
        # Header
        header_frame = ttk.Frame(self.window)
        header_frame.pack(fill=tk.X, padx=10, pady=5)
        
        self.title_label = ttk.Label(header_frame, text="Dictionary Step 1 / 7", 
                                      font=("Helvetica", 12, "bold"))
        self.title_label.pack()
        
        # Text area
        text_frame = ttk.Frame(self.window)
        text_frame.pack(fill=tk.BOTH, expand=True, padx=10, pady=5)
        
        scroll = ttk.Scrollbar(text_frame)
        scroll.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.text_area = tk.Text(text_frame, wrap=tk.WORD, font=("Consolas", 13),
                                  yscrollcommand=scroll.set)
        self.text_area.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        scroll.config(command=self.text_area.yview)
        
        # Navigation buttons
        nav_frame = ttk.Frame(self.window)
        nav_frame.pack(fill=tk.X, padx=10, pady=10)
        
        self.btn_prev = ttk.Button(nav_frame, text="◀ Previous", command=self.prev_step)
        self.btn_prev.pack(side=tk.LEFT, padx=5)
        
        # Progress bar
        self.progress_var = tk.StringVar(value="Step 1 of 7")
        ttk.Label(nav_frame, textvariable=self.progress_var).pack(side=tk.LEFT, expand=True)
        
        self.btn_next = ttk.Button(nav_frame, text="Next ▶", command=self.next_step)
        self.btn_next.pack(side=tk.RIGHT, padx=5)
        
        self.update_display()
        
    def update_display(self):
        if 0 <= self.current_index < len(self.dictionaries):
            self.text_area.delete(1.0, tk.END)
            self.text_area.insert(1.0, self.dictionaries[self.current_index])
            
            self.title_label.config(text=f"Dictionary Step {self.current_index + 1} / {len(self.dictionaries)}")
            self.progress_var.set(f"Step {self.current_index + 1} of {len(self.dictionaries)}")
            
            # Update button states
            self.btn_prev.config(state=tk.NORMAL if self.current_index > 0 else tk.DISABLED)
            self.btn_next.config(state=tk.NORMAL if self.current_index < len(self.dictionaries) - 1 else tk.DISABLED)
    
    def prev_step(self):
        if self.current_index > 0:
            self.current_index -= 1
            self.update_display()
    
    def next_step(self):
        if self.current_index < len(self.dictionaries) - 1:
            self.current_index += 1
            self.update_display()


class LinearProgrammingGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Linear Programming Solver - GUI")
        self.root.geometry("1200x800")
        self.root.minsize(900, 600)
        
        # Find solver path
        base_dir = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
        self.solver_path = os.path.join(base_dir, "src", "LPInterface.exe")
        if not os.path.exists(self.solver_path):
            self.solver_path = os.path.join(base_dir, "src", "LPInterface")
        
        # Variables
        self.num_vars = tk.IntVar(value=2)
        self.num_constraints = tk.IntVar(value=2)
        self.goal = tk.StringVar(value="MAX")
        self.pivot_rule = tk.StringVar(value="DANTZIG")
        
        # Data storage
        self.obj_entries = []
        self.obj_constant_entry = None
        self.constraint_entries = []
        self.constraint_vars = []
        self.relation_vars = []
        self.var_type_vars = []
        
        # Stored data
        self._stored_obj = []
        self._stored_obj_constant = 0
        self._stored_A = []
        self._stored_b = []
        self._stored_relations = []
        self._stored_var_types = []
        
        # Results
        self.last_result = None
        self.last_path = []
        self.last_obj_constant = 0
        self._viz_canvas = None
        self.last_dictionaries = []
        self.current_dict_step = 0
        self._dict_play_after_id = None
        
        self._setup_styles()
        self._create_widgets()
        
    def _setup_styles(self):
        style = ttk.Style()
        style.theme_use('clam')
        style.configure("Title.TLabel", font=("Helvetica", 14, "bold"), foreground="#2c3e50")
        style.configure("Custom.TButton", font=("Helvetica", 10), padding=5)
        style.configure("Solve.TButton", font=("Helvetica", 11, "bold"), padding=8)
        
    def _create_widgets(self):
        # Main frame
        main_frame = ttk.Frame(self.root)
        main_frame.pack(fill=tk.BOTH, expand=True)
        
        # Top section: Title + Setup + Input
        top_frame = ttk.Frame(main_frame)
        top_frame.pack(side=tk.TOP, fill=tk.BOTH, expand=True)
        
        # Title
        title_label = ttk.Label(top_frame, text="Linear Programming Solver", style="Title.TLabel")
        title_label.pack(pady=(10, 5))
        
        # Setup frame
        setup_frame = ttk.LabelFrame(top_frame, text="Problem Setup", padding=5)
        setup_frame.pack(fill=tk.X, padx=10, pady=(0, 5))
        
        # Setup row 1
        row1 = ttk.Frame(setup_frame)
        row1.pack(fill=tk.X, padx=5, pady=2)
        
        ttk.Label(row1, text="Variables:").pack(side=tk.LEFT, padx=(0, 5))
        ttk.Spinbox(row1, from_=1, to=20, textvariable=self.num_vars, width=5, 
                   command=self._on_param_change).pack(side=tk.LEFT, padx=(0, 15))
        
        ttk.Label(row1, text="Constraints:").pack(side=tk.LEFT, padx=(0, 5))
        ttk.Spinbox(row1, from_=1, to=20, textvariable=self.num_constraints, width=5,
                   command=self._on_param_change).pack(side=tk.LEFT, padx=(0, 15))
        
        ttk.Label(row1, text="Goal:").pack(side=tk.LEFT, padx=(0, 5))
        ttk.Radiobutton(row1, text="Max", variable=self.goal, value="MAX").pack(side=tk.LEFT)
        ttk.Radiobutton(row1, text="Min", variable=self.goal, value="MIN").pack(side=tk.LEFT, padx=(0, 15))
        
        ttk.Label(row1, text="Pivot:").pack(side=tk.LEFT, padx=(0, 5))
        ttk.Radiobutton(row1, text="Dantzig", variable=self.pivot_rule, value="DANTZIG").pack(side=tk.LEFT)
        ttk.Radiobutton(row1, text="Bland", variable=self.pivot_rule, value="BLAND").pack(side=tk.LEFT)
        
        # Paned window for resizable panels
        paned = ttk.PanedWindow(top_frame, orient=tk.HORIZONTAL)
        paned.pack(fill=tk.BOTH, expand=True, padx=10, pady=(5, 5))
        
        # Left panel - Input
        input_frame = ttk.Frame(paned)
        paned.add(input_frame, weight=1)
        
        # Objective function
        obj_frame = ttk.LabelFrame(input_frame, text="Objective Function: Z = c₁x₁ + c₂x₂ + ... + d", padding=5)
        obj_frame.pack(fill=tk.X, padx=(0, 5), pady=(0, 5))
        
        self.obj_container = ttk.Frame(obj_frame)
        self.obj_container.pack()
        
        # Constraints
        const_frame = ttk.LabelFrame(input_frame, text="Constraints", padding=5)
        const_frame.pack(fill=tk.BOTH, expand=True, padx=(0, 5), pady=(0, 5))
        
        self.constraint_container = ttk.Frame(const_frame)
        self.constraint_container.pack(fill=tk.BOTH, expand=True)
        
        # Variable Signs (separate section below constraints)
        var_sign_frame = ttk.LabelFrame(input_frame, text="Variable Signs", padding=5)
        var_sign_frame.pack(fill=tk.X, padx=(0, 5))
        
        self.var_sign_container = ttk.Frame(var_sign_frame)
        self.var_sign_container.pack()
        
        # Right panel - Results
        output_frame = ttk.Frame(paned)
        paned.add(output_frame, weight=2)
        
        # Notebook for results
        notebook = ttk.Notebook(output_frame)
        notebook.pack(fill=tk.BOTH, expand=True)
        
        # Solution tab
        solution_tab = ttk.Frame(notebook)
        notebook.add(solution_tab, text="Solution")
        
        result_scroll = ttk.Scrollbar(solution_tab)
        result_scroll.pack(side=tk.RIGHT, fill=tk.Y)
        
        self.result_text = tk.Text(solution_tab, wrap=tk.WORD, font=("Courier", 10), yscrollcommand=result_scroll.set)
        self.result_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        result_scroll.config(command=self.result_text.yview)
        
        # Dictionary tab - with navigation buttons and Consolas font
        dict_tab = ttk.Frame(notebook)
        notebook.add(dict_tab, text="Dictionary")
        
        # Navigation frame at top
        nav_frame = ttk.Frame(dict_tab)
        nav_frame.pack(side=tk.TOP, fill=tk.X, padx=5, pady=3)
        
        self.dict_step_label = ttk.Label(nav_frame, text="Step 0/0", font=("Helvetica", 10))
        self.dict_step_label.pack(side=tk.LEFT, padx=10)
        
        ttk.Button(nav_frame, text="◄ Prev", command=self._dict_prev_step).pack(side=tk.LEFT, padx=2)
        ttk.Button(nav_frame, text="Next ►", command=self._dict_next_step).pack(side=tk.LEFT, padx=2)
        ttk.Button(nav_frame, text="Auto Play", command=self._dict_auto_play).pack(side=tk.LEFT, padx=5)
        
        # Text area with scrollbars
        dict_content_frame = ttk.Frame(dict_tab)
        dict_content_frame.pack(fill=tk.BOTH, expand=True)
        
        dict_scroll_y = ttk.Scrollbar(dict_content_frame)
        dict_scroll_y.pack(side=tk.RIGHT, fill=tk.Y)
        
        dict_scroll_x = ttk.Scrollbar(dict_content_frame, orient=tk.HORIZONTAL)
        dict_scroll_x.pack(side=tk.BOTTOM, fill=tk.X)
        
        self.dict_text = tk.Text(dict_content_frame, wrap=tk.NONE, font=("Consolas", 13),
                                xscrollcommand=dict_scroll_x.set, yscrollcommand=dict_scroll_y.set)
        self.dict_text.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        
        dict_scroll_x.config(command=self.dict_text.xview)
        dict_scroll_y.config(command=self.dict_text.yview)
        
        # Visualization tab
        self.viz_frame = ttk.Frame(notebook)
        notebook.add(self.viz_frame, text="Visualization")
        ttk.Label(self.viz_frame, text="Solve a 2-variable problem to see visualization", 
                  font=("Helvetica", 10)).pack(pady=20)
        
        # Bottom section: Buttons + Status (always visible at bottom)
        bottom_frame = ttk.Frame(main_frame)
        bottom_frame.pack(side=tk.BOTTOM, fill=tk.X, padx=10, pady=5)
        
        # Buttons
        btn_frame = ttk.Frame(bottom_frame)
        btn_frame.pack(side=tk.LEFT)
        
        ttk.Button(btn_frame, text="Solve", style="Solve.TButton", command=self._solve).pack(side=tk.LEFT, padx=3)
        ttk.Button(btn_frame, text="Dictionary", style="Custom.TButton", command=self._show_dictionary).pack(side=tk.LEFT, padx=3)
        ttk.Button(btn_frame, text="Clear", style="Custom.TButton", command=self._clear).pack(side=tk.LEFT, padx=3)
        ttk.Button(btn_frame, text="Example", style="Custom.TButton", command=self._load_example).pack(side=tk.LEFT, padx=3)
        
        # Status bar
        self.status_var = tk.StringVar(value="Ready")
        status_bar = ttk.Label(bottom_frame, textvariable=self.status_var, relief=tk.SUNKEN, anchor=tk.W)
        status_bar.pack(side=tk.RIGHT, fill=tk.X, expand=True, padx=(10, 0))
        
        self._build_input_tables()
        
    def _on_param_change(self):
        self._store_current_values()
        self._build_input_tables()
        
    def _store_current_values(self):
        try:
            self._stored_obj = []
            for row in self.obj_entries:
                for entry in row:
                    try:
                        self._stored_obj.append(float(entry.get()))
                    except:
                        self._stored_obj.append(0)
            
            if self.obj_constant_entry:
                try:
                    self._stored_obj_constant = float(self.obj_constant_entry.get())
                except:
                    self._stored_obj_constant = 0
            
            self._stored_A = []
            for row in self.constraint_entries:
                A_row = []
                for entry in row:
                    try:
                        A_row.append(float(entry.get()))
                    except:
                        A_row.append(0)
                self._stored_A.append(A_row)
            
            self._stored_b = []
            for entry in self.constraint_vars:
                try:
                    self._stored_b.append(float(entry.get()))
                except:
                    self._stored_b.append(0)
            
            self._stored_relations = [var.get() for var in self.relation_vars]
            self._stored_var_types = [var.get() for var in self.var_type_vars]
        except:
            pass
        
    def _restore_values(self, n_vars, n_const):
        for i in range(min(n_vars, len(self._stored_obj))):
            if i < len(self.obj_entries) and i < len(self.obj_entries[0]):
                self.obj_entries[0][i].delete(0, tk.END)
                self.obj_entries[0][i].insert(0, str(self._stored_obj[i]))
        
        if self.obj_constant_entry and self._stored_obj_constant != 0:
            self.obj_constant_entry.delete(0, tk.END)
            self.obj_constant_entry.insert(0, str(self._stored_obj_constant))
        
        for c in range(min(n_const, len(self._stored_A))):
            for i in range(min(n_vars, len(self._stored_A[c]))):
                if c < len(self.constraint_entries) and i < len(self.constraint_entries[c]):
                    self.constraint_entries[c][i].delete(0, tk.END)
                    self.constraint_entries[c][i].insert(0, str(self._stored_A[c][i]))
        
        for c in range(min(n_const, len(self._stored_b))):
            if c < len(self.constraint_vars):
                self.constraint_vars[c].delete(0, tk.END)
                self.constraint_vars[c].insert(0, str(self._stored_b[c]))
        
        for c in range(min(n_const, len(self._stored_relations))):
            if c < len(self.relation_vars):
                self.relation_vars[c].set(self._stored_relations[c])
        
        # Restore variable signs
        n_vars = self.num_vars.get()
        for i in range(min(n_vars, len(self._stored_var_types))):
            if i < len(self.var_type_vars):
                self.var_type_vars[i].set(self._stored_var_types[i])
    
    def _build_input_tables(self):
        for widget in self.obj_container.winfo_children():
            widget.destroy()
        for widget in self.constraint_container.winfo_children():
            widget.destroy()
        for widget in self.var_sign_container.winfo_children():
            widget.destroy()
            
        self.obj_entries = []
        self.obj_constant_entry = None
        self.constraint_entries = []
        self.constraint_vars = []
        self.relation_vars = []
        self.var_type_vars = []
        
        n_vars = self.num_vars.get()
        n_const = self.num_constraints.get()
        
        # Objective function header
        header_frame = ttk.Frame(self.obj_container)
        header_frame.pack(fill=tk.X)
        
        ttk.Label(header_frame, text="Var:", font=("Helvetica", 9, "bold"), width=5).pack(side=tk.LEFT, padx=(0, 2))
        for i in range(n_vars):
            ttk.Label(header_frame, text=f"x{i+1}", font=("Helvetica", 9, "bold"), width=7).pack(side=tk.LEFT, padx=1)
        ttk.Label(header_frame, text="Const(d)", font=("Helvetica", 9, "bold"), width=8).pack(side=tk.LEFT, padx=(3, 0))
        
        # Coefficient row
        entry_frame = ttk.Frame(self.obj_container)
        entry_frame.pack(fill=tk.X, pady=2)
        
        ttk.Label(entry_frame, text="c:", width=5).pack(side=tk.LEFT, padx=(0, 2))
        
        obj_row = []
        for i in range(n_vars):
            entry = ttk.Entry(entry_frame, width=9, justify='center')
            entry.insert(0, "0")
            entry.pack(side=tk.LEFT, padx=1)
            obj_row.append(entry)
        self.obj_entries.append(obj_row)
        
        # Constant term
        constant_entry = ttk.Entry(entry_frame, width=10, justify='center')
        constant_entry.insert(0, "0")
        constant_entry.pack(side=tk.LEFT, padx=(3, 0))
        self.obj_constant_entry = constant_entry
        
        # Variable Signs - each variable on separate row
        for i in range(n_vars):
            var_row_frame = ttk.Frame(self.var_sign_container)
            var_row_frame.pack(fill=tk.X, pady=1)
            
            ttk.Label(var_row_frame, text=f"x{i + 1}:", font=("Helvetica", 9, "bold"), width=4).pack(side=tk.LEFT, padx=(0, 5))
            
            var_type = tk.StringVar(value="≥0")
            
            ttk.Radiobutton(var_row_frame, text="≥0", variable=var_type, value="≥0").pack(side=tk.LEFT, padx=5)
            ttk.Radiobutton(var_row_frame, text="≤0", variable=var_type, value="≤0").pack(side=tk.LEFT, padx=5)
            ttk.Radiobutton(var_row_frame, text="Free", variable=var_type, value="Free").pack(side=tk.LEFT, padx=5)
            
            self.var_type_vars.append(var_type)
        
        # Constraints header
        const_header = ttk.Frame(self.constraint_container)
        const_header.pack(fill=tk.X)
        
        ttk.Label(const_header, text="#", font=("Helvetica", 9, "bold"), width=4).pack(side=tk.LEFT, padx=2)
        for i in range(n_vars):
            ttk.Label(const_header, text=f"x{i+1}", font=("Helvetica", 9, "bold"), width=7).pack(side=tk.LEFT, padx=1)
        ttk.Label(const_header, text="Rel", font=("Helvetica", 9, "bold"), width=5).pack(side=tk.LEFT, padx=1)
        ttk.Label(const_header, text="RHS", font=("Helvetica", 9, "bold"), width=8).pack(side=tk.LEFT, padx=1)
        
        for c in range(n_const):
            row_frame = ttk.Frame(self.constraint_container)
            row_frame.pack(fill=tk.X, pady=1)
            
            ttk.Label(row_frame, text=f"#{c+1}", width=4).pack(side=tk.LEFT, padx=2)
            
            row_entries = []
            for i in range(n_vars):
                entry = ttk.Entry(row_frame, width=9, justify='center')
                entry.insert(0, "0")
                entry.pack(side=tk.LEFT, padx=1)
                row_entries.append(entry)
            self.constraint_entries.append(row_entries)
            
            rel_var = tk.StringVar(value="≤")
            rel_combo = ttk.Combobox(row_frame, textvariable=rel_var, values=["≤", "≥", "="], width=4, state='readonly')
            rel_combo.pack(side=tk.LEFT, padx=1)
            self.relation_vars.append(rel_var)
            
            rhs_entry = ttk.Entry(row_frame, width=10, justify='center')
            rhs_entry.insert(0, "0")
            rhs_entry.pack(side=tk.LEFT, padx=1)
            self.constraint_vars.append(rhs_entry)
        
        if self._stored_obj:
            self._restore_values(n_vars, n_const)
            
    def _load_example(self):
        self.num_vars.set(2)
        self.num_constraints.set(3)
        self.goal.set("MAX")
        self.pivot_rule.set("DANTZIG")
        self._build_input_tables()
        
        self.obj_entries[0][0].delete(0, tk.END)
        self.obj_entries[0][0].insert(0, "3")
        self.obj_entries[0][1].delete(0, tk.END)
        self.obj_entries[0][1].insert(0, "2")
        self.obj_constant_entry.delete(0, tk.END)
        self.obj_constant_entry.insert(0, "5")
        
        constraints = [
            ["2", "1", "≤", "4"],
            ["1", "2", "≤", "12"],
            ["1", "1", "≤", "8"]
        ]
        
        for c, const in enumerate(constraints):
            self.constraint_entries[c][0].delete(0, tk.END)
            self.constraint_entries[c][0].insert(0, const[0])
            self.constraint_entries[c][1].delete(0, tk.END)
            self.constraint_entries[c][1].insert(0, const[1])
            self.relation_vars[c].set(const[2])
            self.constraint_vars[c].delete(0, tk.END)
            self.constraint_vars[c].insert(0, const[3])
        
        self.status_var.set("Example loaded!")
        
    def _clear(self):
        for row in self.obj_entries:
            for entry in row:
                entry.delete(0, tk.END)
                entry.insert(0, "0")
        
        if self.obj_constant_entry:
            self.obj_constant_entry.delete(0, tk.END)
            self.obj_constant_entry.insert(0, "0")
        
        for row in self.constraint_entries:
            for entry in row:
                entry.delete(0, tk.END)
                entry.insert(0, "0")
        
        for rhs_entry in self.constraint_vars:
            rhs_entry.delete(0, tk.END)
            rhs_entry.insert(0, "0")
            
        for var_type in self.var_type_vars:
            var_type.set("≥0")
            
        self.result_text.delete(1.0, tk.END)
        self.dict_text.delete(1.0, tk.END)
        self.last_result = None
        self.last_path = []
        self._clear_visualization()
        self.status_var.set("Cleared!")
        
    def _clear_visualization(self):
        for widget in self.viz_frame.winfo_children():
            widget.destroy()
        ttk.Label(self.viz_frame, text="Solve a 2-variable problem to see visualization", 
                  font=("Helvetica", 10)).pack(pady=20)
        self._viz_canvas = None
        
    def _get_input_data(self) -> Optional[Dict]:
        try:
            obj = [float(entry.get()) for row in self.obj_entries for entry in row]
            obj_constant = float(self.obj_constant_entry.get()) if self.obj_constant_entry else 0
            A = [[float(entry.get()) for entry in row] for row in self.constraint_entries]
            b = [float(entry.get()) for entry in self.constraint_vars]
            
            relations = []
            for rel_var in self.relation_vars:
                rel = rel_var.get()
                if rel == "≤":
                    relations.append(0)
                elif rel == "≥":
                    relations.append(1)
                else:
                    relations.append(2)
            
            var_types = []
            for var_type in self.var_type_vars:
                vt = var_type.get()
                if vt == "≥0":
                    var_types.append(0)
                elif vt == "≤0":
                    var_types.append(1)
                else:
                    var_types.append(2)
            
            return {
                'goal': self.goal.get(),
                'obj': obj,
                'obj_constant': obj_constant,
                'A': A,
                'b': b,
                'relations': relations,
                'var_types': var_types,
                'pivot_rule': self.pivot_rule.get()
            }
        except ValueError as e:
            messagebox.showerror("Input Error", f"Invalid input: {e}")
            return None
            
    def _solve(self):
        data = self._get_input_data()
        if data is None:
            return
        
        if not os.path.exists(self.solver_path):
            messagebox.showerror("Error", 
                f"Solver not found:\n{self.solver_path}\n\n"
                "Please run install.bat first!")
            self.status_var.set("Solver not found!")
            return
        
        self.status_var.set("Solving...")
        self.root.update()
        
        try:
            obj_str = ",".join(str(x) for x in data['obj'])
            matrix_rows = "|".join(",".join(str(x) for x in row) for row in data['A'])
            b_str = ",".join(str(x) for x in data['b'])
            rel_str = ",".join(str(x) for x in data['relations'])
            type_str = ",".join(str(x) for x in data['var_types'])
            
            cmd = [
                self.solver_path,
                "solve",
                data['goal'],
                obj_str,
                matrix_rows,
                b_str,
                rel_str,
                type_str,
                data['pivot_rule'],
                str(data['obj_constant'])
            ]
            
            result = subprocess.run(cmd, capture_output=True, encoding='utf-8', errors='replace', timeout=60)
            
            output = result.stdout if result.stdout else ""
            stderr = result.stderr if result.stderr else ""
            
            if stderr:
                output = output + "\n\n[STDERR]\n" + stderr
            
            if not output.strip():
                messagebox.showerror("Error", "Solver produced no output.")
                self.status_var.set("No output!")
                return
            
            self.last_obj_constant = data['obj_constant']
            self._parse_and_display_result(output, data)
            self.current_dict_step = 0
            self._update_dict_display()
            self.status_var.set("Solved!")
            
        except FileNotFoundError:
            messagebox.showerror("Error", "Solver not found. Run install.bat first!")
            self.status_var.set("Solver not found!")
        except subprocess.TimeoutExpired:
            messagebox.showerror("Timeout", "Solver took too long.")
            self.status_var.set("Timeout!")
        except Exception as e:
            messagebox.showerror("Error", f"Solver error: {str(e)}")
            self.status_var.set("Error!")
            
    def _parse_and_display_result(self, output: str, data: Dict):
        if not output:
            output = "No output from solver"
        
        # Parse and store dictionaries for step viewer
        self._parse_dictionaries(output)
        
        # Parse solution - just the key info
        solution_lines = []
        
        # Check status
        status_match = re.search(r'=== SOLVER_STATUS ===\s*(\w+)', output)
        status = status_match.group(1) if status_match else "UNKNOWN"
        
        if status == "OPTIMAL":
            # Extract variable values
            var_values = {}
            for match in re.finditer(r'x(\d+)\s*=\s*([-\d.]+)', output):
                var_values[int(match.group(1))] = match.group(2)
            
            # Extract optimal value
            z_match = re.search(r'(Max|Min)\s*Z\s*=\s*([-\d.]+)', output)
            z_value = z_match.group(2) if z_match else "?"
            z_type = z_match.group(1) if z_match else "Z"
            
            solution_lines.append("=" * 50)
            solution_lines.append("          KẾT LUẬN NGHIỆM")
            solution_lines.append("=" * 50)
            solution_lines.append("")
            solution_lines.append("[NGHIỆM TỐI ƯU]")
            solution_lines.append("")
            solution_lines.append("Các giá trị biến:")
            solution_lines.append("")
            
            for i in range(1, self.num_vars.get() + 1):
                val = var_values.get(i, "0")
                solution_lines.append(f"  x{i}  =  {val}")
            
            solution_lines.append("")
            solution_lines.append("-" * 50)
            solution_lines.append(f"  Giá trị tối ưu:  {z_type} Z = {z_value}")
            solution_lines.append("=" * 50)
            
        elif status == "INFEASIBLE":
            solution_lines.append("=" * 50)
            solution_lines.append("          KẾT LUẬN NGHIỆM")
            solution_lines.append("=" * 50)
            solution_lines.append("")
            solution_lines.append("[VÔ NGHIỆM]")
            solution_lines.append("")
            solution_lines.append("Miền chấp nhận được là miền rỗng.")
            solution_lines.append("=" * 50)
            
        elif status == "UNBOUNDED":
            solution_lines.append("=" * 50)
            solution_lines.append("          KẾT LUẬN NGHIỆM")
            solution_lines.append("=" * 50)
            solution_lines.append("")
            solution_lines.append("[KHÔNG GIỚI NỘI]")
            solution_lines.append("")
            solution_lines.append("Giá trị hàm mục tiêu có thể tăng vô hạn.")
            solution_lines.append("=" * 50)
            
        else:
            solution_lines.append(f"Trạng thái: {status}")
        
        solution_text = "\n".join(solution_lines)
        self.result_text.delete(1.0, tk.END)
        self.result_text.insert(1.0, solution_text)
        
        # Parse path from GUI_PATH output
        self.last_path = []
        path_match = re.search(r'=== GUI_PATH ===\s*([\d.,;-]+)', output)
        if path_match:
            path_str = path_match.group(1)
            for point_str in path_str.split(';'):
                coords = point_str.strip().split(',')
                if len(coords) >= 2:
                    try:
                        x1 = float(coords[0])
                        x2 = float(coords[1])
                        self.last_path.append((x1, x2))
                    except ValueError:
                        pass
        
        # Parse status
        status_match = re.search(r'=== SOLVER_STATUS ===\s*(\w+)', output)
        if status_match:
            self.last_result = status_match.group(1)
        
        if self.num_vars.get() == 2 and len(self.last_path) >= 1:
            self._show_visualization(data)
        else:
            self._clear_visualization()
            
    def _show_dictionary(self):
        data = self._get_input_data()
        if data is None:
            return
        
        if not os.path.exists(self.solver_path):
            messagebox.showerror("Error", "Solver not found. Run install.bat first!")
            return
            
        self.status_var.set("Generating dictionary...")
        
        try:
            obj_str = ",".join(str(x) for x in data['obj'])
            matrix_rows = "|".join(",".join(str(x) for x in row) for row in data['A'])
            b_str = ",".join(str(x) for x in data['b'])
            rel_str = ",".join(str(x) for x in data['relations'])
            type_str = ",".join(str(x) for x in data['var_types'])
            
            cmd = [
                self.solver_path,
                "dictionary",
                data['goal'],
                obj_str,
                matrix_rows,
                b_str,
                rel_str,
                type_str
            ]
            
            result = subprocess.run(cmd, capture_output=True, encoding='utf-8', errors='replace', timeout=30)
            output = result.stdout if result.stdout else ""
            stderr = result.stderr if result.stderr else ""
            
            if not output.strip():
                if stderr:
                    messagebox.showerror("Error", f"Solver error:\n{stderr}")
                else:
                    messagebox.showerror("Error", "Solver produced no output.\n\nCheck if solver compiled correctly.")
                self.status_var.set("No output!")
                return
            
            # Show stderr as warning if present
            if stderr and stderr.strip():
                output = output + "\n\n[STDERR]\n" + stderr
            
            # Parse and store dictionaries for step viewer
            self._parse_dictionaries(output)
            self.current_dict_step = 0
            
            self._update_dict_display()
            self.status_var.set("Dictionary generated!")
            
        except Exception as e:
            messagebox.showerror("Error", f"Dictionary error: {str(e)}")
            self.status_var.set("Error!")
    
    def _parse_dictionaries(self, output: str):
        """Parse solver output into individual dictionaries"""
        self.last_dictionaries = []
        
        # Split by "[ Iteration #N ]" markers - include the bracket
        import re
        pattern = r'(\[\s*Iteration #\d+\s*\].*?)(?=\[\s*Iteration #|\n=== SOLVER_STATUS ===|\Z)'
        matches = re.findall(pattern, output, re.DOTALL)
        
        if matches:
            for match in matches:
                match = match.strip()
                if match:
                    self.last_dictionaries.append(match)
        else:
            # If no iteration markers, try splitting by "[ Iteration #"
            parts = re.split(r'(\[\s*Iteration #\d+)', output)
            current_dict = ""
            for i, part in enumerate(parts):
                if part.startswith("[ Iteration #"):
                    if current_dict:
                        self.last_dictionaries.append(current_dict.strip())
                    current_dict = part
                else:
                    current_dict += part
            if current_dict:
                self.last_dictionaries.append(current_dict.strip())
        
        # If still empty, just use the whole output
        if not self.last_dictionaries:
            self.last_dictionaries = [output] if output.strip() else []
    
    def _open_step_viewer(self):
        """Open step-by-step dictionary viewer"""
        if not self.last_dictionaries:
            messagebox.showinfo("No Data", "Please run 'Solve' or 'Dictionary' first!")
            return
        
        if len(self.last_dictionaries) == 1:
            messagebox.showinfo("Single Step", "Only one iteration. Use the Dictionary tab instead.")
            return
        
        # Switch to Dictionary tab
        notebook = None
        for widget in self.root.winfo_children():
            if isinstance(widget, ttk.PanedWindow):
                for child in widget.winfo_children():
                    if isinstance(child, ttk.Frame):
                        for w in child.winfo_children():
                            if isinstance(w, ttk.Notebook):
                                notebook = w
                                break
        
        if notebook:
            for i, tab_id in enumerate(notebook.tabs()):
                if notebook.tab(tab_id, "text") == "Dictionary":
                    notebook.select(i)
                    break
        
        self._update_dict_display()
            
    def _dict_prev_step(self):
        """Go to previous dictionary step"""
        if not self.last_dictionaries:
            return
        self.current_dict_step = max(0, self.current_dict_step - 1)
        self._update_dict_display()
        
    def _dict_next_step(self):
        """Go to next dictionary step"""
        if not self.last_dictionaries:
            return
        self.current_dict_step = min(len(self.last_dictionaries) - 1, self.current_dict_step + 1)
        self._update_dict_display()
        
    def _dict_auto_play(self):
        """Auto play through dictionary steps"""
        if not self.last_dictionaries:
            return
        
        if self._dict_play_after_id:
            self.root.after_cancel(self._dict_play_after_id)
            self._dict_play_after_id = None
            return
        
        def play_next():
            if self.current_dict_step < len(self.last_dictionaries) - 1:
                self.current_dict_step += 1
                self._update_dict_display()
                self._dict_play_after_id = self.root.after(800, play_next)
            else:
                self._dict_play_after_id = None
        
        play_next()
        
    def _update_dict_display(self):
        """Update dictionary text display for current step"""
        if not self.last_dictionaries:
            self.dict_text.delete(1.0, tk.END)
            self.dict_step_label.config(text="No data")
            return
        
        total = len(self.last_dictionaries)
        step = self.current_dict_step + 1
        self.dict_step_label.config(text=f"Step {step}/{total}")
        
        self.dict_text.delete(1.0, tk.END)
        self.dict_text.insert(1.0, self.last_dictionaries[self.current_dict_step])
            
    def _show_visualization(self, data: Dict):
        self._clear_visualization()
        
        if len(self.last_path) < 1:
            ttk.Label(self.viz_frame, text="No path points found for visualization").pack(pady=20)
            return
        
        try:
            fig = Figure(figsize=(6, 5), dpi=100)
            ax = fig.add_subplot(111)
            
            A = np.array(data['A'])
            b = np.array(data['b'])
            relations = data['relations']
            
            x_max = max(15, max(b) * 1.5 if len(b) > 0 else 15)
            y_max = max(15, max(b) * 1.5 if len(b) > 0 else 15)
            x_min, y_min = 0, 0
            
            x = np.linspace(x_min, x_max, 500)
            y = np.linspace(y_min, y_max, 500)
            X, Y = np.meshgrid(x, y)
            
            feasible = np.ones_like(X, dtype=bool)
            for i in range(len(b)):
                constraint = A[i, 0] * X + A[i, 1] * Y
                if relations[i] == 0:
                    feasible = feasible & (constraint <= b[i] + 0.001)
                elif relations[i] == 1:
                    feasible = feasible & (constraint >= b[i] - 0.001)
                else:
                    feasible = feasible & (np.abs(constraint - b[i]) < 0.001)
            
            feasible = feasible & (X >= 0) & (Y >= 0)
            
            ax.contourf(X, Y, feasible.astype(int), levels=[0.5, 1], colors=['lightblue'], alpha=0.3)
            ax.contour(X, Y, feasible.astype(int), levels=[0.5], colors=['blue'], linewidths=2)
            
            colors = ['#e74c3c', '#2ecc71', '#9b59b6', '#f39c12', '#1abc9c', '#e67e22', '#3498db', '#c0392b']
            
            x_line = np.linspace(x_min, x_max, 200)
            for i in range(len(b)):
                color = colors[i % len(colors)]
                if abs(A[i, 1]) > 1e-9:
                    y_const = (b[i] - A[i, 0] * x_line) / A[i, 1]
                    mask = (y_const >= y_min) & (y_const <= y_max)
                    rel_symbol = ["≤", "≥", "="][relations[i]]
                    ax.plot(x_line[mask], y_const[mask], color=color, linewidth=2, 
                           label=f"#{i+1}: {A[i,0]:.1f}x₁ + {A[i,1]:.1f}x₂ {rel_symbol} {b[i]}")
                elif abs(A[i, 0]) > 1e-9:
                    ax.axvline(x=b[i]/A[i, 0], color=color, linewidth=2, linestyle='-',
                              label=f"#{i+1}: {A[i,0]:.1f}x₁ {['≤','≥','='][relations[i]]} {b[i]}")
            
            ax.axhline(y=0, color='k', linewidth=0.5)
            ax.axvline(x=0, color='k', linewidth=0.5)
            
            if len(self.last_path) > 1:
                path_x = [p[0] for p in self.last_path]
                path_y = [p[1] for p in self.last_path]
                
                # Draw line
                ax.plot(path_x, path_y, 'r-', linewidth=2, alpha=0.7, zorder=3)
                
                # Draw small arrows between points
                from matplotlib.patches import FancyArrowPatch
                for i in range(len(path_x) - 1):
                    arrow = FancyArrowPatch(
                        (path_x[i], path_y[i]),
                        (path_x[i+1], path_y[i+1]),
                        arrowstyle='->', mutation_scale=15,
                        color='red', linewidth=2, zorder=4
                    )
                    ax.add_patch(arrow)
                
                # Markers (small dots)
                ax.scatter(path_x, path_y, color='red', s=50, zorder=5)
            
            ax.set_xlabel('x₁', fontsize=12)
            ax.set_ylabel('x₂', fontsize=12)
            
            goal_str = "Maximize" if data['goal'] == 'MAX' else "Minimize"
            const_str = f" + {data['obj_constant']}" if data['obj_constant'] != 0 else ""
            obj_str = " + ".join([f"{c}x{i+1}" for i, c in enumerate(data['obj'])])
            ax.set_title(f'{goal_str} Z = {obj_str}{const_str}', fontsize=11)
            
            ax.set_xlim(x_min, x_max)
            ax.set_ylim(y_min, y_max)
            ax.grid(True, alpha=0.3)
            ax.set_aspect('equal')
            
            canvas = FigureCanvasTkAgg(fig, master=self.viz_frame)
            canvas.draw()
            canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
            self._viz_canvas = canvas
            
            if self.last_result == 'OPTIMAL' and len(self.last_path) > 0:
                opt_point = self.last_path[-1]
                opt_value = sum(data['obj'][i] * opt_point[i] for i in range(len(data['obj'])))
                opt_value += data['obj_constant']
                info_text = f"Optimal: x₁ = {opt_point[0]:.3f}, x₂ = {opt_point[1]:.3f}, Z = {opt_value:.3f}"
            else:
                opt_value = 0
                info_text = f"Status: {self.last_result}"
                
            ttk.Label(self.viz_frame, text=info_text, font=("Helvetica", 10, "bold")).pack(pady=5)
            
        except Exception as e:
            ttk.Label(self.viz_frame, text=f"Visualization error: {str(e)}").pack(pady=20)


def main():
    root = tk.Tk()
    app = LinearProgrammingGUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()
