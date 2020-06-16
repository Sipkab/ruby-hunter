#ifndef RESCONFIG_H_
#define RESCONFIG_H_

#include <framework/utils/FixedString.h>

#include <gen/configuration.h>
#include <gen/log.h>

namespace rhfw {

namespace configuration {
class ConfigurationManager;
}

class ResourceConfiguration;
void applyPlatformConfigurations(ResourceConfiguration& config);

class ResourceConfiguration {
	friend class ResourceManager;
	friend class configuration::ConfigurationManager;
private:
	//static ResourceConfiguration current;

	@replace:resconfig_private_members@

	unsigned int version;
	unsigned long long changedMask;

	class ConfigurationValue {
	public:
		union {
			unsigned int integer;
			float flp;
		};
		const char* pointer;

		bool operator==(const FixedString& o) const {
			if (integer != o.length())
				return false;
			for (unsigned int i = 0; i < integer; ++i) {
				if (pointer[i] != o[i])
					return false;
			}
			return true;
		}
	};

	void setAtIndex(unsigned int index, const ConfigurationValue& val) {
		@replace:resconfig_setatindex@
	}
	ResourceConfiguration(int) { //placehoder for static member initialization
	}
public:
	static void initStatic() {
		LOGTRACE();
		@replace:resconfig_initstatic@
		//current.version = 0;
		//current.changedMask = 0xFFFFFFFFFFFFFFFF;

		//applyPlatformConfigurations(current);
	}
	static void destroyStatic() {
		LOGTRACE();
		@replace:resconfig_destroystatic@
	}

	ResourceConfiguration()
			/*: ResourceConfiguration(current)*/ {
		changedMask = 0x0;
	}
	ResourceConfiguration(const ResourceConfiguration&) = default;
	ResourceConfiguration(ResourceConfiguration&&) = default;
	ResourceConfiguration& operator=(const ResourceConfiguration&) = default;
	ResourceConfiguration& operator=(ResourceConfiguration&&) = default;
	@replace:resconfig_public_members@

	unsigned long long getChangeMask(const ResourceConfiguration& other) const {
		unsigned long long result = 0;
		@replace:resconfig_getchangemask@
		return result;
	}

	bool matches(const ResourceConfiguration& other, unsigned long long mask) const {
		@replace:resconfig_matches@
		return true;
	}
	bool matchesAtIndex(unsigned int index, const ConfigurationValue& val) const {
		@replace:resconfig_matchesatindex@
		return false;
	}
};

}

#endif /*RESCONFIG_H_*/
