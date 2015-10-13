/*
 * Author Tim Diels <timdiels.m@gmail.com>
 *
 * From: http://www.codeproject.com/Tips/884269/Generating-Unique-Contrasting-Colors-in-VB-NET
 * By Gregory Morse
 *
 * Translated to C++
 */

#include <deep_blue_genome/coexpr/stdafx.h>
#include "color.h"

using namespace std;

namespace DEEP_BLUE_GENOME {
namespace UTIL {
/*
	Public Structure XYZColor
        Public X As Double
        Public Y As Double
        Public Z As Double
    End Structure
    Public Shared WhiteReference As New XYZColor With {.X = 95.047, .Y = 100.0, .Z = 108.883}
    Public Const Epsilon As Double = 0.008856 'Intent is 216/24389
    Public Const Kappa As Double = 903.3 'Intent is 24389/27
    Public Shared Function PivotRGB(N As Double) As Double
        Return If(N > 0.04045, Math.Pow((N + 0.055) / 1.055, 2.4), N / 12.92) * 100.0
    End Function
    Public Shared Function ToRGB(N As Double) As Double
        Dim Result As Double = N * 255.0
        If Result < 0 Then Return 0
        If Result > 255 Then Return 255
        Return Result
    End Function
    Public Shared Function RGBToXYZ(clr As Color) As XYZColor
        Dim r As Double = PivotRGB(clr.R / 255.0)
        Dim g As Double = PivotRGB(clr.G / 255.0)
        Dim b As Double = PivotRGB(clr.B / 255.0)
        Return New XYZColor With {.X = r * 0.4124 + g * 0.3576 + b * 0.1805, _
                                  .Y = r * 0.2126 + g * 0.7152 + b * 0.0722, _
                                  .Z = r * 0.0193 + g * 0.1192 + b * 0.9505}
    End Function
    Public Shared Function XYZToRGB(clr As XYZColor) As Color
        Dim x As Double = clr.X / 100.0
        Dim y As Double = clr.Y / 100.0
        Dim z As Double = clr.Z / 100.0
        Dim r As Double = x * 3.2406 + y * -1.5372 + z * -0.4986
        Dim g As Double = x * -0.9689 + y * 1.8758 + z * 0.0415
        Dim b As Double = x * 0.0557 + y * -0.204 + z * 1.057
        r = If(r > 0.0031308, 1.055 * Math.Pow(r, 1 / 2.4) - 0.055, 12.92 * r)
        g = If(g > 0.0031308, 1.055 * Math.Pow(g, 1 / 2.4) - 0.055, 12.92 * g)
        b = If(b > 0.0031308, 1.055 * Math.Pow(b, 1 / 2.4) - 0.055, 12.92 * b)
        Return Color.FromArgb(CInt(ToRGB(r)), CInt(ToRGB(g)), CInt(ToRGB(b)))
    End Function
    Public Structure LABColor
        Public L As Double
        Public A As Double
        Public B As Double
    End Structure
    Public Shared Function PivotXYZ(N As Double) As Double
        Return If(N > Epsilon, Math.Pow(N, 1.0 / 3.0), (Kappa * N + 16) / 116)
    End Function
    Public Shared Function RGBToLAB(clr As Color) As LABColor
        Dim XYZCol As XYZColor = RGBToXYZ(clr)
        Dim x As Double = PivotXYZ(XYZCol.X / WhiteReference.X)
        Dim y As Double = PivotXYZ(XYZCol.Y / WhiteReference.Y)
        Dim z As Double = PivotXYZ(XYZCol.Z / WhiteReference.Z)
        Return New LABColor With {.L = Math.Max(0, 116 * y - 16), .A = 500 * (x - y), .B = 200 * (y - z)}
    End Function
    Public Shared Function LABToRGB(clr As LABColor) As Color
        Dim y As Double = (clr.L + 16.0) / 116.0
        Dim x As Double = clr.A / 500.0 + y
        Dim z As Double = y - clr.B / 200.0
        Dim X3 As Double = x * x * x
        Dim Z3 As Double = z * z * z
        Return XYZToRGB(New XYZColor With {.X = WhiteReference.X * _
        If(X3 > Epsilon, X3, (x - 16.0 / 116.0) / 7.787), _
                                         .Y = WhiteReference.Y * _
                                         If(clr.L > (Kappa * Epsilon), _
                                         Math.Pow((clr.L + 16.0) / 116.0, 3), _
                                         clr.L / Kappa), _
                                         .Z = WhiteReference.Z * If(Z3 > _
                                         Epsilon, Z3, (z - 16.0 / 116.0) / 7.787)})
    End Function
    Public Shared Function CMCCompareColors_
    (ColorA As LABColor, ColorB As LABColor, Lightness As Double, Chroma As Double) As Double
        Dim deltaL As Double = ColorA.L - ColorB.L
        Dim h As Double = Math.Atan2(ColorB.B, ColorA.A)
        Dim C1 As Double = Math.Sqrt(ColorA.A * ColorA.A + ColorA.B * ColorA.B)
        Dim C2 As Double = Math.Sqrt(ColorB.A * ColorB.A + ColorB.B * ColorB.B)
        Dim deltaC As Double = C1 - C2
        Dim deltaH As Double = Math.Sqrt((ColorA.A - ColorB.A) * _
        (ColorA.A - ColorB.A) + (ColorA.B - ColorB.B) * (ColorA.B - ColorB.B) - deltaC * deltaC)
        Dim C1_4 As Double = C1 * C1
        C1_4 *= C1_4
        Dim t As Double = If(164 <= h Or h >= 345, 0.56 + _
        Math.Abs(0.2 * Math.Cos(h + 168.0)), 0.36 + Math.Abs(0.4 * Math.Cos(h + 35.0)))
        Dim f As Double = Math.Sqrt(C1_4 / (C1_4 + 1900.0))
        Dim sL As Double = If(ColorA.L < 16, 0.511, _
        (0.040975 * ColorA.L) / (1.0 + 0.01765 * ColorA.L))
        Dim sC As Double = (0.0638 * C1) / (1 + 0.0131 * C1) + 0.638
        Dim sH As Double = sC * (f * t + 1 - f)
        Return Math.Sqrt(deltaL * deltaL / (Lightness * Lightness * sL * sL) + _
        deltaC * deltaC / (Chroma * Chroma * sC * sC) + deltaH * deltaH / (sH * sH))
    End Function
    Public Shared Function GCD(A As Integer, B As Integer) As Integer 'Euclid's algorithm
        If B = 0 Then Return A
        Return GCD(B, A Mod B)
    End Function
    Public Shared Function GenerateNDistinctColors_
    (N As Integer, Threshold As Integer, Interleave As Integer) As Color()
        'To best support individuals with colorblindness _
        (deuteranopia or protanopia) keep a set to 0; vary only L and b.
        Dim LABColors As New List(Of LABColor)
        Dim LowThresholds As New List(Of Double)
        LABColors.Add(RGBToLAB(Color.Black)) 'Start with pivot forecolor
        LowThresholds.Add(100)
        LABColors.Add(RGBToLAB(Color.White)) 'Start with background color
        LowThresholds.Add(100)
        For A = 0 To 200 'Pivot around 0 and move towards 100/-100
            For L = 0 To 100 / 2 'dark to light yet for readability do not exceed half of the spectrum
                For B = 0 To 200 'Pivot around 0 and move towards 100/-100
                    Dim CurColCount As Integer
                    Dim LowThreshold As Double = 100
                    For CurColCount = 0 To LABColors.Count - 1
                        LowThreshold = Math.Min(LowThreshold, CMCCompareColors_
                        (LABColors(CurColCount), New LABColor With {.L = L, .A = ((A \ 2) + _
                        If((A Mod 2) = 1, 1, 0)) * If((A Mod 2) = 1, 1, -1), .B = ((B \ 2) + _
                        If((B Mod 2) = 1, 1, 0)) * If((B Mod 2) = 1, 1, -1)}, 1.0, 1.0))
                        If LowThreshold < Threshold Then Exit For
                    Next
                    If CurColCount = LABColors.Count Then
                        Dim Idx As Integer = LowThresholds.BinarySearch(LowThreshold)
                        If Idx < 0 Then Idx = Idx Xor -1
                        If Idx <> 0 Or LowThresholds.Count <> N + 1 Then
                            LABColors.Insert(Idx, New LABColor With {.L = L, .A = ((A \ 2) + _
                            If((A Mod 2) = 1, 1, 0)) * If((A Mod 2) = 1, 1, -1), .B = ((B \ 2) + _
                            If((B Mod 2) = 1, 1, 0)) * If((B Mod 2) = 1, 1, -1)})
                            LowThresholds.Insert(Idx, LowThreshold)
                            If LowThresholds.Count > N + 1 Then
                                LABColors.RemoveAt(0)
                                LowThresholds.RemoveAt(0)
                            End If
                        End If
                    End If
                Next
            Next
            If LABColors.Count >= N + 1 Then Exit For
        Next
        LABColors.RemoveAt(N) 'Remove background color
        'if less than N colors found then try with lower threshold
        If LABColors.Count < N Then Return GenerateNDistinctColors(N, Threshold - 1, Interleave)
        Dim Cols(N - 1) As Color
        For Count As Integer = 0 To N - 1
            'Least Common Multiple LCM = a * b \ GCD(A, B)
            Cols((Count * Interleave) \ (N * Interleave \ GCD(N, Interleave)) + _
            (Count * Interleave) Mod N) = LABToRGB(LABColors(Count))
        Next
        Return Cols
    End Function
*/
}} // end namespace
