# Install script for directory: C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/ABC1")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mbedtls" TYPE FILE PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ WORLD_READ FILES
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aes.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aesni.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/arc4.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/aria.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/asn1write.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/base64.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bignum.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/blowfish.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/bn_mul.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/camellia.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ccm.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/certs.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chacha20.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/chachapoly.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/check_config.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cipher_internal.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/cmac.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/compat-1.3.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/config.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ctr_drbg.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/debug.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/des.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/dhm.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdh.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecdsa.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecjpake.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ecp_internal.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/entropy_poll.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/error.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/gcm.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/havege.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hkdf.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/hmac_drbg.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md2.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md4.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md5.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/md_internal.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/memory_buffer_alloc.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/net_sockets.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/nist_kw.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/oid.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/padlock.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pem.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pk_internal.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs11.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs12.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/pkcs5.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_time.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/platform_util.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/poly1305.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ripemd160.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/rsa_internal.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha1.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha256.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/sha512.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cache.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ciphersuites.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_cookie.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_internal.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/ssl_ticket.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/threading.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/timing.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/version.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crl.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_crt.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/x509_csr.h"
    "C:/msys32/home/mahesh_tripathi/esp/esp-idf/components/mbedtls/mbedtls/include/mbedtls/xtea.h"
    )
endif()

