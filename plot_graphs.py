import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import os

# Data for Task 20
data_20 = {
    'Version': ['CPU', 'GPU_Basic', 'GPU_Opt', 'CPU', 'GPU_Basic', 'GPU_Opt'],
    'Size': [1000, 1000, 1000, 4000, 4000, 4000],
    'SolveTime_s': [0.598, 0.830, 0.008, 17.510, 7.284, 0.131],
    'TotalTime_s': [0.626, 0.859, 0.351, 18.000, 7.809, 0.994]
}
df_20 = pd.DataFrame(data_20)

# Data for Task 21
df_21 = pd.read_csv('/home/luiz/Documents/alta-computacao-tarefas/21-tarefa/results.csv')

def plot_bar_chart(df, title, filename):
    sizes = df['Size'].unique()
    versions = df['Version'].unique()
    
    x = np.arange(len(sizes))
    width = 0.25
    
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    
    # Solve Time
    for i, version in enumerate(versions):
        subset = df[df['Version'] == version]
        axes[0].bar(x + i*width - width, subset['SolveTime_s'], width, label=version)
    
    axes[0].set_xlabel('Tamanho da Malha (N)')
    axes[0].set_ylabel('Tempo de Compute (s)')
    axes[0].set_title(f'Tempo de Compute por Tamanho da Malha ({title})')
    axes[0].set_xticks(x)
    axes[0].set_xticklabels(sizes)
    axes[0].legend()
    axes[0].grid(axis='y', linestyle='--', alpha=0.7)
    
    # Total Time
    for i, version in enumerate(versions):
        subset = df[df['Version'] == version]
        axes[1].bar(x + i*width - width, subset['TotalTime_s'], width, label=version)
    
    axes[1].set_xlabel('Tamanho da Malha (N)')
    axes[1].set_ylabel('Tempo Total (s)')
    axes[1].set_title(f'Tempo Total por Tamanho da Malha ({title})')
    axes[1].set_xticks(x)
    axes[1].set_xticklabels(sizes)
    axes[1].legend()
    axes[1].grid(axis='y', linestyle='--', alpha=0.7)
    
    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    plt.close()
    print(f"Salvo {filename}")

def plot_line_chart(df, title, filename):
    fig, axes = plt.subplots(1, 2, figsize=(14, 6))
    versions = df['Version'].unique()
    
    for version in versions:
        subset = df[df['Version'] == version]
        axes[0].plot(subset['Size'], subset['SolveTime_s'], marker='o', label=version)
        axes[1].plot(subset['Size'], subset['TotalTime_s'], marker='o', label=version)
        
    axes[0].set_xlabel('Tamanho da Malha (N)')
    axes[0].set_ylabel('Tempo de Compute (s)')
    axes[0].set_title(f'Escalabilidade do Tempo de Compute ({title})')
    axes[0].set_xscale('log', base=2)
    axes[0].set_yscale('log', base=10)
    axes[0].legend()
    axes[0].grid(True, which="both", ls="--")
    
    axes[1].set_xlabel('Tamanho da Malha (N)')
    axes[1].set_ylabel('Tempo Total (s)')
    axes[1].set_title(f'Escalabilidade do Tempo Total ({title})')
    axes[1].set_xscale('log', base=2)
    axes[1].set_yscale('log', base=10)
    axes[1].legend()
    axes[1].grid(True, which="both", ls="--")
    
    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    plt.close()
    print(f"Salvo {filename}")

# Fix specific task 21 names if needed
df_21['Version'] = df_21['Version'].replace({'GPU_Basic': 'GPU Basic', 'GPU_Opt': 'GPU Optimized'})
df_20['Version'] = df_20['Version'].replace({'GPU_Basic': 'GPU Basic', 'GPU_Opt': 'GPU Optimized'})

plot_bar_chart(df_20, 'Tarefa 20', '/home/luiz/Documents/alta-computacao-tarefas/tarefa_20_bar_chart.png')
plot_bar_chart(df_21, 'Tarefa 21', '/home/luiz/Documents/alta-computacao-tarefas/tarefa_21_bar_chart.png')
plot_line_chart(df_21, 'Tarefa 21 (Log Scale)', '/home/luiz/Documents/alta-computacao-tarefas/tarefa_21_line_chart.png')
