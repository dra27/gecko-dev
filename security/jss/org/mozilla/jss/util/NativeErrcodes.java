/* ***** BEGIN LICENSE BLOCK *****
 * Version: MPL 1.1/GPL 2.0/LGPL 2.1
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Original Code is Network Security Services for Java.
 *
 * The Initial Developer of the Original Code is
 * Netscape Communications Corporation.
 * Portions created by the Initial Developer are Copyright (C) 2002
 * the Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 2 or later (the "GPL"), or
 * the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
 * in which case the provisions of the GPL or the LGPL are applicable instead
 * of those above. If you wish to allow use of your version of this file only
 * under the terms of either the GPL or the LGPL, and not to allow others to
 * use your version of this file under the terms of the MPL, indicate your
 * decision by deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL or the LGPL. If you do not delete
 * the provisions above, a recipient may use your version of this file under
 * the terms of any one of the MPL, the GPL or the LGPL.
 *
 * ***** END LICENSE BLOCK ***** */


package org.mozilla.jss.util;

/**
 An enumeration of all error codes from NSS and NSPR.
 The integer values associated with each error code are subject to change, so
 <b>DO NOT DEPEND ON THEM</b>. Only use the symbolic names.
*/
public class NativeErrcodes {

    public static final int PR_OUT_OF_MEMORY_ERROR = 1;
    public static final int PR_BAD_DESCRIPTOR_ERROR = 2;
    public static final int PR_WOULD_BLOCK_ERROR = 3;
    public static final int PR_ACCESS_FAULT_ERROR = 4;
    public static final int PR_INVALID_METHOD_ERROR = 5;
    public static final int PR_ILLEGAL_ACCESS_ERROR = 6;
    public static final int PR_UNKNOWN_ERROR = 7;
    public static final int PR_PENDING_INTERRUPT_ERROR = 8;
    public static final int PR_NOT_IMPLEMENTED_ERROR = 9;
    public static final int PR_IO_ERROR = 10;
    public static final int PR_IO_TIMEOUT_ERROR = 11;
    public static final int PR_IO_PENDING_ERROR = 12;
    public static final int PR_DIRECTORY_OPEN_ERROR = 13;
    public static final int PR_INVALID_ARGUMENT_ERROR = 14;
    public static final int PR_ADDRESS_NOT_AVAILABLE_ERROR = 15;
    public static final int PR_ADDRESS_NOT_SUPPORTED_ERROR = 16;
    public static final int PR_IS_CONNECTED_ERROR = 17;
    public static final int PR_BAD_ADDRESS_ERROR = 18;
    public static final int PR_ADDRESS_IN_USE_ERROR = 19;
    public static final int PR_CONNECT_REFUSED_ERROR = 20;
    public static final int PR_NETWORK_UNREACHABLE_ERROR = 21;
    public static final int PR_CONNECT_TIMEOUT_ERROR = 22;
    public static final int PR_NOT_CONNECTED_ERROR = 23;
    public static final int PR_LOAD_LIBRARY_ERROR = 24;
    public static final int PR_UNLOAD_LIBRARY_ERROR = 25;
    public static final int PR_FIND_SYMBOL_ERROR = 26;
    public static final int PR_INSUFFICIENT_RESOURCES_ERROR = 27;
    public static final int PR_DIRECTORY_LOOKUP_ERROR = 28;
    public static final int PR_TPD_RANGE_ERROR = 29;
    public static final int PR_PROC_DESC_TABLE_FULL_ERROR = 30;
    public static final int PR_SYS_DESC_TABLE_FULL_ERROR = 31;
    public static final int PR_NOT_SOCKET_ERROR = 32;
    public static final int PR_NOT_TCP_SOCKET_ERROR = 33;
    public static final int PR_SOCKET_ADDRESS_IS_BOUND_ERROR = 34;
    public static final int PR_NO_ACCESS_RIGHTS_ERROR = 35;
    public static final int PR_OPERATION_NOT_SUPPORTED_ERROR = 36;
    public static final int PR_PROTOCOL_NOT_SUPPORTED_ERROR = 37;
    public static final int PR_REMOTE_FILE_ERROR = 38;
    public static final int PR_BUFFER_OVERFLOW_ERROR = 39;
    public static final int PR_CONNECT_RESET_ERROR = 40;
    public static final int PR_RANGE_ERROR = 41;
    public static final int PR_DEADLOCK_ERROR = 42;
    public static final int PR_FILE_IS_LOCKED_ERROR = 43;
    public static final int PR_FILE_TOO_BIG_ERROR = 44;
    public static final int PR_NO_DEVICE_SPACE_ERROR = 45;
    public static final int PR_PIPE_ERROR = 46;
    public static final int PR_NO_SEEK_DEVICE_ERROR = 47;
    public static final int PR_IS_DIRECTORY_ERROR = 48;
    public static final int PR_LOOP_ERROR = 49;
    public static final int PR_NAME_TOO_LONG_ERROR = 50;
    public static final int PR_FILE_NOT_FOUND_ERROR = 51;
    public static final int PR_NOT_DIRECTORY_ERROR = 52;
    public static final int PR_READ_ONLY_FILESYSTEM_ERROR = 53;
    public static final int PR_DIRECTORY_NOT_EMPTY_ERROR = 54;
    public static final int PR_FILESYSTEM_MOUNTED_ERROR = 55;
    public static final int PR_NOT_SAME_DEVICE_ERROR = 56;
    public static final int PR_DIRECTORY_CORRUPTED_ERROR = 57;
    public static final int PR_FILE_EXISTS_ERROR = 58;
    public static final int PR_MAX_DIRECTORY_ENTRIES_ERROR = 59;
    public static final int PR_INVALID_DEVICE_STATE_ERROR = 60;
    public static final int PR_DEVICE_IS_LOCKED_ERROR = 61;
    public static final int PR_NO_MORE_FILES_ERROR = 62;
    public static final int PR_END_OF_FILE_ERROR = 63;
    public static final int PR_FILE_SEEK_ERROR = 64;
    public static final int PR_FILE_IS_BUSY_ERROR = 65;
    public static final int PR_OPERATION_ABORTED_ERROR = 66;
    public static final int PR_IN_PROGRESS_ERROR = 67;
    public static final int PR_ALREADY_INITIATED_ERROR = 68;
    public static final int PR_GROUP_EMPTY_ERROR = 69;
    public static final int PR_INVALID_STATE_ERROR = 70;
    public static final int PR_NETWORK_DOWN_ERROR = 71;
    public static final int PR_SOCKET_SHUTDOWN_ERROR = 72;
    public static final int PR_CONNECT_ABORTED_ERROR = 73;
    public static final int PR_HOST_UNREACHABLE_ERROR = 74;
    public static final int SSL_ERROR_BASE = 77;
    public static final int SSL_ERROR_EXPORT_ONLY_SERVER = 78;
    public static final int SSL_ERROR_US_ONLY_SERVER = 79;
    public static final int SSL_ERROR_NO_CYPHER_OVERLAP = 80;
    public static final int SSL_ERROR_NO_CERTIFICATE = 81;
    public static final int SSL_ERROR_BAD_CERTIFICATE = 82;
    public static final int SSL_ERROR_BAD_CLIENT = 83;
    public static final int SSL_ERROR_BAD_SERVER = 84;
    public static final int SSL_ERROR_UNSUPPORTED_CERTIFICATE_TYPE = 85;
    public static final int SSL_ERROR_UNSUPPORTED_VERSION = 86;
    public static final int SSL_ERROR_WRONG_CERTIFICATE = 87;
    public static final int SSL_ERROR_BAD_CERT_DOMAIN = 88;
    public static final int SSL_ERROR_POST_WARNING = 89;
    public static final int SSL_ERROR_SSL2_DISABLED = 90;
    public static final int SSL_ERROR_BAD_MAC_READ = 91;
    public static final int SSL_ERROR_BAD_MAC_ALERT = 92;
    public static final int SSL_ERROR_BAD_CERT_ALERT = 93;
    public static final int SSL_ERROR_REVOKED_CERT_ALERT = 94;
    public static final int SSL_ERROR_EXPIRED_CERT_ALERT = 95;
    public static final int SSL_ERROR_SSL_DISABLED = 96;
    public static final int SSL_ERROR_FORTEZZA_PQG = 97;
    public static final int SSL_ERROR_UNKNOWN_CIPHER_SUITE = 98;
    public static final int SSL_ERROR_NO_CIPHERS_SUPPORTED = 99;
    public static final int SSL_ERROR_BAD_BLOCK_PADDING = 100;
    public static final int SSL_ERROR_RX_RECORD_TOO_LONG = 101;
    public static final int SSL_ERROR_TX_RECORD_TOO_LONG = 102;
    public static final int SSL_ERROR_RX_MALFORMED_HELLO_REQUEST = 103;
    public static final int SSL_ERROR_RX_MALFORMED_CLIENT_HELLO = 104;
    public static final int SSL_ERROR_RX_MALFORMED_SERVER_HELLO = 105;
    public static final int SSL_ERROR_RX_MALFORMED_CERTIFICATE = 106;
    public static final int SSL_ERROR_RX_MALFORMED_SERVER_KEY_EXCH = 107;
    public static final int SSL_ERROR_RX_MALFORMED_CERT_REQUEST = 108;
    public static final int SSL_ERROR_RX_MALFORMED_HELLO_DONE = 109;
    public static final int SSL_ERROR_RX_MALFORMED_CERT_VERIFY = 110;
    public static final int SSL_ERROR_RX_MALFORMED_CLIENT_KEY_EXCH = 111;
    public static final int SSL_ERROR_RX_MALFORMED_FINISHED = 112;
    public static final int SSL_ERROR_RX_MALFORMED_CHANGE_CIPHER = 113;
    public static final int SSL_ERROR_RX_MALFORMED_ALERT = 114;
    public static final int SSL_ERROR_RX_MALFORMED_HANDSHAKE = 115;
    public static final int SSL_ERROR_RX_MALFORMED_APPLICATION_DATA = 116;
    public static final int SSL_ERROR_RX_UNEXPECTED_HELLO_REQUEST = 117;
    public static final int SSL_ERROR_RX_UNEXPECTED_CLIENT_HELLO = 118;
    public static final int SSL_ERROR_RX_UNEXPECTED_SERVER_HELLO = 119;
    public static final int SSL_ERROR_RX_UNEXPECTED_CERTIFICATE = 120;
    public static final int SSL_ERROR_RX_UNEXPECTED_SERVER_KEY_EXCH = 121;
    public static final int SSL_ERROR_RX_UNEXPECTED_CERT_REQUEST = 122;
    public static final int SSL_ERROR_RX_UNEXPECTED_HELLO_DONE = 123;
    public static final int SSL_ERROR_RX_UNEXPECTED_CERT_VERIFY = 124;
    public static final int SSL_ERROR_RX_UNEXPECTED_CLIENT_KEY_EXCH = 125;
    public static final int SSL_ERROR_RX_UNEXPECTED_FINISHED = 126;
    public static final int SSL_ERROR_RX_UNEXPECTED_CHANGE_CIPHER = 127;
    public static final int SSL_ERROR_RX_UNEXPECTED_ALERT = 128;
    public static final int SSL_ERROR_RX_UNEXPECTED_HANDSHAKE = 129;
    public static final int SSL_ERROR_RX_UNEXPECTED_APPLICATION_DATA = 130;
    public static final int SSL_ERROR_RX_UNKNOWN_RECORD_TYPE = 131;
    public static final int SSL_ERROR_RX_UNKNOWN_HANDSHAKE = 132;
    public static final int SSL_ERROR_RX_UNKNOWN_ALERT = 133;
    public static final int SSL_ERROR_CLOSE_NOTIFY_ALERT = 134;
    public static final int SSL_ERROR_HANDSHAKE_UNEXPECTED_ALERT = 135;
    public static final int SSL_ERROR_DECOMPRESSION_FAILURE_ALERT = 136;
    public static final int SSL_ERROR_HANDSHAKE_FAILURE_ALERT = 137;
    public static final int SSL_ERROR_ILLEGAL_PARAMETER_ALERT = 138;
    public static final int SSL_ERROR_UNSUPPORTED_CERT_ALERT = 139;
    public static final int SSL_ERROR_CERTIFICATE_UNKNOWN_ALERT = 140;
    public static final int SSL_ERROR_GENERATE_RANDOM_FAILURE = 141;
    public static final int SSL_ERROR_SIGN_HASHES_FAILURE = 142;
    public static final int SSL_ERROR_EXTRACT_PUBLIC_KEY_FAILURE = 143;
    public static final int SSL_ERROR_SERVER_KEY_EXCHANGE_FAILURE = 144;
    public static final int SSL_ERROR_CLIENT_KEY_EXCHANGE_FAILURE = 145;
    public static final int SSL_ERROR_ENCRYPTION_FAILURE = 146;
    public static final int SSL_ERROR_DECRYPTION_FAILURE = 147;
    public static final int SSL_ERROR_SOCKET_WRITE_FAILURE = 148;
    public static final int SSL_ERROR_MD5_DIGEST_FAILURE = 149;
    public static final int SSL_ERROR_SHA_DIGEST_FAILURE = 150;
    public static final int SSL_ERROR_MAC_COMPUTATION_FAILURE = 151;
    public static final int SSL_ERROR_SYM_KEY_CONTEXT_FAILURE = 152;
    public static final int SSL_ERROR_SYM_KEY_UNWRAP_FAILURE = 153;
    public static final int SSL_ERROR_PUB_KEY_SIZE_LIMIT_EXCEEDED = 154;
    public static final int SSL_ERROR_IV_PARAM_FAILURE = 155;
    public static final int SSL_ERROR_INIT_CIPHER_SUITE_FAILURE = 156;
    public static final int SSL_ERROR_SESSION_KEY_GEN_FAILURE = 157;
    public static final int SSL_ERROR_NO_SERVER_KEY_FOR_ALG = 158;
    public static final int SSL_ERROR_TOKEN_INSERTION_REMOVAL = 159;
    public static final int SSL_ERROR_TOKEN_SLOT_NOT_FOUND = 160;
    public static final int SSL_ERROR_NO_COMPRESSION_OVERLAP = 161;
    public static final int SSL_ERROR_HANDSHAKE_NOT_COMPLETED = 162;
    public static final int SSL_ERROR_BAD_HANDSHAKE_HASH_VALUE = 163;
    public static final int SSL_ERROR_CERT_KEA_MISMATCH = 164;
    public static final int SSL_ERROR_NO_TRUSTED_SSL_CLIENT_CA = 165;
    public static final int SSL_ERROR_SESSION_NOT_FOUND = 166;
    public static final int SSL_ERROR_DECRYPTION_FAILED_ALERT = 167;
    public static final int SSL_ERROR_RECORD_OVERFLOW_ALERT = 168;
    public static final int SSL_ERROR_UNKNOWN_CA_ALERT = 169;
    public static final int SSL_ERROR_ACCESS_DENIED_ALERT = 170;
    public static final int SSL_ERROR_DECODE_ERROR_ALERT = 171;
    public static final int SSL_ERROR_DECRYPT_ERROR_ALERT = 172;
    public static final int SSL_ERROR_EXPORT_RESTRICTION_ALERT = 173;
    public static final int SSL_ERROR_PROTOCOL_VERSION_ALERT = 174;
    public static final int SSL_ERROR_INSUFFICIENT_SECURITY_ALERT = 175;
    public static final int SSL_ERROR_INTERNAL_ERROR_ALERT = 176;
    public static final int SSL_ERROR_USER_CANCELED_ALERT = 177;
    public static final int SSL_ERROR_NO_RENEGOTIATION_ALERT = 178;
    public static final int SEC_ERROR_IO = 179;
    public static final int SEC_ERROR_LIBRARY_FAILURE = 180;
    public static final int SEC_ERROR_BAD_DATA = 181;
    public static final int SEC_ERROR_OUTPUT_LEN = 182;
    public static final int SEC_ERROR_INPUT_LEN = 183;
    public static final int SEC_ERROR_INVALID_ARGS = 184;
    public static final int SEC_ERROR_INVALID_ALGORITHM = 185;
    public static final int SEC_ERROR_INVALID_AVA = 186;
    public static final int SEC_ERROR_INVALID_TIME = 187;
    public static final int SEC_ERROR_BAD_DER = 188;
    public static final int SEC_ERROR_BAD_SIGNATURE = 189;
    public static final int SEC_ERROR_EXPIRED_CERTIFICATE = 190;
    public static final int SEC_ERROR_REVOKED_CERTIFICATE = 191;
    public static final int SEC_ERROR_UNKNOWN_ISSUER = 192;
    public static final int SEC_ERROR_BAD_KEY = 193;
    public static final int SEC_ERROR_BAD_PASSWORD = 194;
    public static final int SEC_ERROR_RETRY_PASSWORD = 195;
    public static final int SEC_ERROR_NO_NODELOCK = 196;
    public static final int SEC_ERROR_BAD_DATABASE = 197;
    public static final int SEC_ERROR_NO_MEMORY = 198;
    public static final int SEC_ERROR_UNTRUSTED_ISSUER = 199;
    public static final int SEC_ERROR_UNTRUSTED_CERT = 200;
    public static final int SEC_ERROR_DUPLICATE_CERT = 201;
    public static final int SEC_ERROR_DUPLICATE_CERT_NAME = 202;
    public static final int SEC_ERROR_ADDING_CERT = 203;
    public static final int SEC_ERROR_FILING_KEY = 204;
    public static final int SEC_ERROR_NO_KEY = 205;
    public static final int SEC_ERROR_CERT_VALID = 206;
    public static final int SEC_ERROR_CERT_NOT_VALID = 207;
    public static final int SEC_ERROR_CERT_NO_RESPONSE = 208;
    public static final int SEC_ERROR_EXPIRED_ISSUER_CERTIFICATE = 209;
    public static final int SEC_ERROR_CRL_EXPIRED = 210;
    public static final int SEC_ERROR_CRL_BAD_SIGNATURE = 211;
    public static final int SEC_ERROR_CRL_INVALID = 212;
    public static final int SEC_ERROR_EXTENSION_VALUE_INVALID = 213;
    public static final int SEC_ERROR_EXTENSION_NOT_FOUND = 214;
    public static final int SEC_ERROR_CA_CERT_INVALID = 215;
    public static final int SEC_ERROR_PATH_LEN_CONSTRAINT_INVALID = 216;
    public static final int SEC_ERROR_CERT_USAGES_INVALID = 217;
    public static final int SEC_INTERNAL_ONLY = 218;
    public static final int SEC_ERROR_INVALID_KEY = 219;
    public static final int SEC_ERROR_UNKNOWN_CRITICAL_EXTENSION = 220;
    public static final int SEC_ERROR_OLD_CRL = 221;
    public static final int SEC_ERROR_NO_EMAIL_CERT = 222;
    public static final int SEC_ERROR_NO_RECIPIENT_CERTS_QUERY = 223;
    public static final int SEC_ERROR_NOT_A_RECIPIENT = 224;
    public static final int SEC_ERROR_PKCS7_KEYALG_MISMATCH = 225;
    public static final int SEC_ERROR_PKCS7_BAD_SIGNATURE = 226;
    public static final int SEC_ERROR_UNSUPPORTED_KEYALG = 227;
    public static final int SEC_ERROR_DECRYPTION_DISALLOWED = 228;
    public static final int XP_SEC_FORTEZZA_BAD_CARD = 229;
    public static final int XP_SEC_FORTEZZA_NO_CARD = 230;
    public static final int XP_SEC_FORTEZZA_NONE_SELECTED = 231;
    public static final int XP_SEC_FORTEZZA_MORE_INFO = 232;
    public static final int XP_SEC_FORTEZZA_PERSON_NOT_FOUND = 233;
    public static final int XP_SEC_FORTEZZA_NO_MORE_INFO = 234;
    public static final int XP_SEC_FORTEZZA_BAD_PIN = 235;
    public static final int XP_SEC_FORTEZZA_PERSON_ERROR = 236;
    public static final int SEC_ERROR_NO_KRL = 237;
    public static final int SEC_ERROR_KRL_EXPIRED = 238;
    public static final int SEC_ERROR_KRL_BAD_SIGNATURE = 239;
    public static final int SEC_ERROR_REVOKED_KEY = 240;
    public static final int SEC_ERROR_KRL_INVALID = 241;
    public static final int SEC_ERROR_NEED_RANDOM = 242;
    public static final int SEC_ERROR_NO_MODULE = 243;
    public static final int SEC_ERROR_NO_TOKEN = 244;
    public static final int SEC_ERROR_READ_ONLY = 245;
    public static final int SEC_ERROR_NO_SLOT_SELECTED = 246;
    public static final int SEC_ERROR_CERT_NICKNAME_COLLISION = 247;
    public static final int SEC_ERROR_KEY_NICKNAME_COLLISION = 248;
    public static final int SEC_ERROR_SAFE_NOT_CREATED = 249;
    public static final int SEC_ERROR_BAGGAGE_NOT_CREATED = 250;
    public static final int XP_JAVA_REMOVE_PRINCIPAL_ERROR = 251;
    public static final int XP_JAVA_DELETE_PRIVILEGE_ERROR = 252;
    public static final int XP_JAVA_CERT_NOT_EXISTS_ERROR = 253;
    public static final int SEC_ERROR_BAD_EXPORT_ALGORITHM = 254;
    public static final int SEC_ERROR_EXPORTING_CERTIFICATES = 255;
    public static final int SEC_ERROR_IMPORTING_CERTIFICATES = 256;
    public static final int SEC_ERROR_PKCS12_DECODING_PFX = 257;
    public static final int SEC_ERROR_PKCS12_INVALID_MAC = 258;
    public static final int SEC_ERROR_PKCS12_UNSUPPORTED_MAC_ALGORITHM = 259;
    public static final int SEC_ERROR_PKCS12_UNSUPPORTED_TRANSPORT_MODE = 260;
    public static final int SEC_ERROR_PKCS12_CORRUPT_PFX_STRUCTURE = 261;
    public static final int SEC_ERROR_PKCS12_UNSUPPORTED_PBE_ALGORITHM = 262;
    public static final int SEC_ERROR_PKCS12_UNSUPPORTED_VERSION = 263;
    public static final int SEC_ERROR_PKCS12_PRIVACY_PASSWORD_INCORRECT = 264;
    public static final int SEC_ERROR_PKCS12_CERT_COLLISION = 265;
    public static final int SEC_ERROR_USER_CANCELLED = 266;
    public static final int SEC_ERROR_PKCS12_DUPLICATE_DATA = 267;
    public static final int SEC_ERROR_MESSAGE_SEND_ABORTED = 268;
    public static final int SEC_ERROR_INADEQUATE_KEY_USAGE = 269;
    public static final int SEC_ERROR_INADEQUATE_CERT_TYPE = 270;
    public static final int SEC_ERROR_CERT_ADDR_MISMATCH = 271;
    public static final int SEC_ERROR_PKCS12_UNABLE_TO_IMPORT_KEY = 272;
    public static final int SEC_ERROR_PKCS12_IMPORTING_CERT_CHAIN = 273;
    public static final int SEC_ERROR_PKCS12_UNABLE_TO_LOCATE_OBJECT_BY_NAME = 274;
    public static final int SEC_ERROR_PKCS12_UNABLE_TO_EXPORT_KEY = 275;
    public static final int SEC_ERROR_PKCS12_UNABLE_TO_WRITE = 276;
    public static final int SEC_ERROR_PKCS12_UNABLE_TO_READ = 277;
    public static final int SEC_ERROR_PKCS12_KEY_DATABASE_NOT_INITIALIZED = 278;
    public static final int SEC_ERROR_KEYGEN_FAIL = 279;
    public static final int SEC_ERROR_INVALID_PASSWORD = 280;
    public static final int SEC_ERROR_RETRY_OLD_PASSWORD = 281;
    public static final int SEC_ERROR_BAD_NICKNAME = 282;
    public static final int SEC_ERROR_NOT_FORTEZZA_ISSUER = 283;
    public static final int SEC_ERROR_CANNOT_MOVE_SENSITIVE_KEY = 284;
    public static final int SEC_ERROR_JS_INVALID_MODULE_NAME = 285;
    public static final int SEC_ERROR_JS_INVALID_DLL = 286;
    public static final int SEC_ERROR_JS_ADD_MOD_FAILURE = 287;
    public static final int SEC_ERROR_JS_DEL_MOD_FAILURE = 288;
    public static final int SEC_ERROR_OLD_KRL = 289;
    public static final int SEC_ERROR_CKL_CONFLICT = 290;
    public static final int SEC_ERROR_CERT_NOT_IN_NAME_SPACE = 291;
    public static final int SEC_ERROR_KRL_NOT_YET_VALID = 292;
    public static final int SEC_ERROR_CRL_NOT_YET_VALID = 293;
    public static final int SEC_ERROR_UNKNOWN_CERT = 294;
    public static final int SEC_ERROR_UNKNOWN_SIGNER = 295;
    public static final int SEC_ERROR_CERT_BAD_ACCESS_LOCATION = 296;
    public static final int SEC_ERROR_OCSP_UNKNOWN_RESPONSE_TYPE = 297;
    public static final int SEC_ERROR_OCSP_BAD_HTTP_RESPONSE = 298;
    public static final int SEC_ERROR_OCSP_MALFORMED_REQUEST = 299;
    public static final int SEC_ERROR_OCSP_SERVER_ERROR = 300;
    public static final int SEC_ERROR_OCSP_TRY_SERVER_LATER = 301;
    public static final int SEC_ERROR_OCSP_REQUEST_NEEDS_SIG = 302;
    public static final int SEC_ERROR_OCSP_UNAUTHORIZED_REQUEST = 303;
    public static final int SEC_ERROR_OCSP_UNKNOWN_RESPONSE_STATUS = 304;
    public static final int SEC_ERROR_OCSP_UNKNOWN_CERT = 305;
    public static final int SEC_ERROR_OCSP_NOT_ENABLED = 306;
    public static final int SEC_ERROR_OCSP_NO_DEFAULT_RESPONDER = 307;
    public static final int SEC_ERROR_OCSP_MALFORMED_RESPONSE = 308;
    public static final int SEC_ERROR_OCSP_UNAUTHORIZED_RESPONSE = 309;
    public static final int SEC_ERROR_OCSP_FUTURE_RESPONSE = 310;
    public static final int SEC_ERROR_OCSP_OLD_RESPONSE = 311;
    public static final int SEC_ERROR_DIGEST_NOT_FOUND = 312;
    public static final int SEC_ERROR_UNSUPPORTED_MESSAGE_TYPE = 313;
}
