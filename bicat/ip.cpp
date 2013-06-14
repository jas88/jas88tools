#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <string>

class IPaddress {
public:
	static IPaddress *get(std::string);
private:
};

class IPfour : public IPaddress {
public:
	IPfour(std::string);
private:
	struct sockaddr_in addr;
};

class IPsix	: public IPaddress {
public:
	IPsix(std::string);
private:
	struct sockaddr_in6 addr;
};


IPaddress *IPaddress::get(std::string a) {
	try {
		return new IPsix(a);
	}
	catch(std::string err) {
		return new IPfour(a);
	}
}


IPfour::IPfour(std::string a) {
	size_t end;
	if ((end=a.find(':'))!=a.npos) {
		a[end]='\0';
		addr.sin_port=htons(atoi(a.substr(end+1).c_str()));
	}
	if (inet_pton(AF_INET,a.substr(1).c_str(),&addr.sin_addr)!=1)
		throw "Invalid IPv4 address";
}

IPsix::IPsix(std::string a) {
	addr.sin6_family=AF_INET6;
	if (a[0]=='[') {
		// [addr]:port
		if (inet_pton(AF_INET6,a.substr(1).c_str(),&addr.sin6_addr)!=1)
			throw "Invalid IPv6 address";
		
		unsigned end=a.find(']');
		a[end]='\0';
		if (a[end+1]==':') {
			// Port number present
			addr.sin6_port=htons(atoi(a.substr(end+2).c_str()));
		}
	}
	
	else {
		if (inet_pton(AF_INET6,a.c_str(),&addr.sin6_addr)!=1)
			throw "Invalid IPv6 address";
	}
}