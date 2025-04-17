# EEET‑425 · Digital Signal Processing Laboratory  
_Repository of MATLAB & Arduino material for the Spring 2025 semester (section 2245)_

---

## 1  Catalog description  
> Develops the knowledge and ability to process signals using Digital Signal Processing (DSP) techniques. Starts with foundational concepts in sampling, probability, statistics, noise, fixed‑ and floating‑point number systems, and describes how they affect the real‑world performance of DSP systems. Fundamental principles of convolution, linearity, duality, impulse responses, and discrete Fourier transforms are used to develop FIR and IIR digital filters and to explain DSP techniques such as windowing. Students get an integrated lab experience writing DSP code that executes in real‑time on DSP hardware.

---

## 2  Course learning outcomes  
After completing the labs and project, a student will be able to:

* Apply statistics to quantify signal‑to‑noise improvement and oversampling‑based resolution gain.  
* Design FIR and IIR filters with several classical techniques.  
* Relate time‑domain impulse response to frequency‑domain behaviour and analyse filter performance in both domains.  
* Select and apply appropriate windowing methods.  
* Perform digital filtering and spectral analysis with MATLAB (and Excel when appropriate).  

---

## 3  Repository layout
```bash
EEET-425
├───Lab
│   ├───FinalProject
│   │   ├───data
│   │   ├───Final_Project_Code_Base
│   │   └───Input Files
│   ├───Lab1
│   ├───Lab2
│   │   └───data
│   ├───Lab3
│   │   └───Codebase
│   │       ├───1.3
│   │       ├───1.4
│   │       ├───2.1
│   │       ├───3a
│   │       ├───3b
│   │       ├───4
│   │       └───5
│   ├───Lab4
│   │   ├───data
│   │   └───DSP_Lab04_CodeBase_2221
│   │       └───MsTimer2-1.1.0
│   │           └───examples
│   │               └───FlashLed
│   ├───Lab5
│   │   └───procedure
│   │       ├───docs
│   │       ├───include
│   │       ├───lib
│   │       ├───section1
│   │       ├───section2
│   │       ├───section6
│   │       ├───src
│   │       │   └───matlab
│   │       │       └───data
│   │       └───test
│   ├───Lab6
│   │   ├───CodeBase_Lab06_Section1_2245
│   │   ├───CodeBase_Lab06_Section2_2245
│   │   ├───CodeBase_Lab06_Section3_2245
│   │   ├───CodeBase_Lab06_Section5_2245
│   │   └───data
│   ├───Lab7
│   │   ├───data
│   │   ├───Lab7_FIR_Section_01_Base_Code
│   │   ├───Lab7_FIR_Section_02_Base_Code
│   │   └───Lab7_FIR_Section_03_Base_Code
│   ├───Lab8
│   │   ├───data
│   │   └───EqualizerTest_Code_Base
│   └───Lab9
│       ├───data
│       └───Lab_09_IIR_Filters_Codebase
└───Lecture
    ├───Exam 2
    ├───Homework 5
    ├───Homework 6
    └───Homework 8
```
---

## 4  Lab topics
The laboratory covers: 

1. Introduction to MATLAB tools & linear drift
2. Using MATLAB to compute signal statistics
3. Introduction to DSP hardware
4. ADC, SNR, Dither, & Standard deviation
5. Data Types, Memory, & Processing Speed
6. Convolution & FIR Filters
7. FIR Filter Design
8. Equalizer Design
9. Higher Order IIR Filters
10. **Final Project:** Arduino-based Breathing Rate Monitor

---

## 5  Getting started

### Software prerequisites
| Tool | Tested version | Notes |
|------|---------------|-------|
| MATLAB | R2024b | Signal Processing & DSP System toolboxes recommended |
| Arduino IDE | ≥ 1.8 | Developed on YourDuino-RoboRed with DSP Kit |

### Quick start
```bash
git clone https://github.com/JRSumner1/EEET-425.git
cd EEET-425
matlab -r "open('Lab/Lab1/Lab1_Procedure.mlx')"
```
