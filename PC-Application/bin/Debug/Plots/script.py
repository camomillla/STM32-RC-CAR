#coding=Windows-1250
import pandas as pd
import matplotlib.pyplot as plt
import os

os.environ["TF_ENABLE_ONEDNN_OPTS"] = "0"
abspath = os.path.abspath(__file__)
dname = os.path.dirname(abspath)
os.chdir(dname)

# Funkcja do wczytania danych z pliku CSV
def read_csv(filename):
    return pd.read_csv(filename)

# Wczytywanie danych z plików CSV
motor_a_data = read_csv("MotorA.csv")
motor_b_data = read_csv("MotorB.csv")
motor_avg_data = read_csv("MotorAvg.csv")

# Tworzenie wykresów
fig, axes = plt.subplots(3, 1, figsize=(10, 15))

# Wykres 1: MotorA
axes[0].plot(motor_a_data['Index'], motor_a_data['MotorA Measured'], label='Prędkość mierzona', color='blue')
axes[0].plot(motor_a_data['Index'], motor_a_data['MotorA Set'], label='Prędkość zadana', color='red')
axes[0].set_title('Prawy silnik')
axes[0].set_xlabel('Próbka')
axes[0].set_ylabel('Prędkość [RPM]')
axes[0].legend()
axes[0].grid()

# Wykres 2: MotorB
axes[1].plot(motor_b_data['Index'], motor_b_data['MotorB Measured'], label='Prędkość mierzona', color='blue')
axes[1].plot(motor_b_data['Index'], motor_b_data['MotorB Set'], label='Prędkość zadana', color='red')
axes[1].set_title('Lewy silnik')
axes[1].set_xlabel('Próbka')
axes[1].set_ylabel('Prędkość [RPM]')
axes[1].legend()
axes[1].grid()

# Wykres 3: Average Speeds
axes[2].plot(motor_avg_data['Index'], motor_avg_data['Avg Measured'], label='Prędkość mierzona', color='blue')
axes[2].plot(motor_avg_data['Index'], motor_avg_data['Avg Set'], label='Prędkość zadana', color='red')
axes[2].set_title('Wartość średnia')
axes[2].set_xlabel('Próbka')
axes[2].set_ylabel('Prędkość [RPM]')
axes[2].legend()
axes[2].grid()

# Ustawienia ogólne wykresów
plt.tight_layout()
plt.subplots_adjust(hspace=0.25)
plt.show()
