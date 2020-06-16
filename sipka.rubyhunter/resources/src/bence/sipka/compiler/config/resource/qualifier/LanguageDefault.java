/*
 * Copyright (C) 2020 Bence Sipka
 *
 * This program is free software: you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation, version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package bence.sipka.compiler.config.resource.qualifier;

@Deprecated
public enum LanguageDefault {
	any(Language.any),
	ar(Language.ar_IL),
	bg(Language.bg_BG),
	ca(Language.ca_ES),
	cs(Language.cs_CZ),
	da(Language.da_DK),
	de(Language.de_DE),
	el(Language.el_GR),
	en(Language.en_US),
	es(Language.es_ES),
	fi(Language.fi_FI),
	fr(Language.fr_FR),
	hi(Language.hi_IN),
	hr(Language.hr_HR),
	hu(Language.hu_HU),
	in(Language.in_ID),
	it(Language.it_IT),
	iw(Language.iw_IL),
	ja(Language.ja_JP),
	ko(Language.ko_KR),
	lt(Language.lt_LT),
	lv(Language.lv_LV),
	nb(Language.nb_NO),
	nl(Language.nl_NL),
	pl(Language.pl_PL),
	pt(Language.pt_PT),
	ro(Language.ro_RO),
	ru(Language.ru_RU),
	sk(Language.sk_SK),
	sl(Language.sl_SI),
	sr(Language.sr_RS),
	sv(Language.sv_SE),
	th(Language.th_TH),
	tl(Language.tl_PH),
	tr(Language.tr_TR),
	uk(Language.uk_UA),
	vi(Language.vi_VN),
	zh(Language.zh_CN);

	private final Language language;

	private LanguageDefault(Language defval) {
		this.language = defval;
	}

	public Language getLanguage() {
		return language;
	}

}
