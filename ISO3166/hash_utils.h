#pragma once

unsigned int MixinHash(
	const std::wstring &wstr,
	unsigned int initval
	);
unsigned int MixinHash(
	const std::string &str,
	unsigned int initval
	);
unsigned int get_token_fingerprint(
	const std::wstring::const_iterator &it_beg,
	const std::wstring::const_iterator &it_end, 
	unsigned int prev_value
	);
unsigned int get_token_fingerprint(
	const wstring &token,
	unsigned int prev_value
	);
unsigned int get_token_fingerprint(
	const string::const_iterator &it_beg,
	const string::const_iterator &it_end, 
	unsigned int prev_value
	);
unsigned int get_token_fingerprint(
	const string &token,
	unsigned int prev_value
	);