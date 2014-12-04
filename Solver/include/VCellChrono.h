#ifndef VCellChrono_h
#define VCellChrono_h
 /* Copyright (C) 2014 UConn Health 
 *
 * Licensed under the MIT License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *  http://www.opensource.org/licenses/mit-license.php
 */
#include <chrono>
#include <iomanip>
namespace vcell_util {
	template <class D> struct HMSDescription;

	namespace HMSFormat {
		/**
		* compact as possible
		*/
		static const int COMPACT = 0;
		/**
		* all (include hours and minutes if 0) 
		*/
		static const int ALL = 1;
		/**
		* zero pad so all the same
		*/
		static const int FIXED = 2; 
	};


	/**
	* @tparam D std::chrono::duration
	*/
	template <class D>
	struct HMS {
		HMS(const D &d, int f = HMSFormat::COMPACT)
			:period(d),
			format(f) {}

		void writeValue(std::ostream &os) const {
			using namespace std::chrono;
			seconds copy = duration_cast<seconds>(period);
			hours h = duration_cast<hours>(copy);
			copy -= h;
			minutes m = duration_cast<minutes>(copy);
			copy -= m;
			int hv = h.count( );
			int mv = m.count( );
			const bool all  = isAll( ); 
			if (all || hv > 0) {
				setformat(os)<< hv << ':'; 
			}
			if (all || mv > 0) {
				setformat(os)<< mv << ':'; 
			}
			setformat(os)<< copy.count( ); 
		}

		void writeDescription(std::ostream &os) const {
			using namespace std::chrono;
			const bool all  = isAll( ); 
			if ( all ||  duration_cast<hours>(period).count( ) > 0)  {
				os << "hours:";
			}
			if ( all || duration_cast<minutes>(period).count( ) > 0) {
				os << "minutes:";
			}
			os << "seconds";
		}
		/**
		* manipulator to describe format
		*/
		HMSDescription<D> describe( ) const {
			return HMSDescription<D>(*this);
		}

	private:
		bool isAll( ) const {
			using HMSFormat::ALL;
			return (format&ALL) == ALL;
		}
		std::ostream &setformat(std::ostream &os) const {
			using HMSFormat::FIXED;
			if ((format&FIXED) == FIXED) {
				os.width(2);
				os.fill('0');
			}
			return os;
		}
		const D &period;
		const int format;

	};

	template <class D>
	struct HMSDescription {
		HMSDescription(const HMS<D> & d)
			:hms(d) {}
		const HMS<D> & hms;
	};


	template <class D>
	std::ostream & operator<<(std::ostream &os, const HMS<D> &hms) {
		hms.writeValue(os);
		return os;
	}

	template <class D>
	std::ostream & operator<<(std::ostream &os, const HMSDescription<D> &desc) {
		desc.hms.writeDescription(os);
		return os;
	}
}
#endif
