#pragma once

#include<string>//ログの文字列を出力するため

//ログを出力する関数
void Log(const std::string& message);

void Log(std::ostream& os, const std::string& message);

//string->wstringに変換する関数
std::wstring ConvertString(const std::string& str);
//wstring->stringに変換する関数
std::string ConvertString(const std::wstring& str);