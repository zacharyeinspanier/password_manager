// Encryption Tests
//
// Checks the ChaCha20-Poly1305 implementation against the official test vectors in RFC 8439,
// then the password format (round trip, tamper detection, wrong key) and the key file rules.
#include <cassert>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "../../app/encryption/encryption.cpp"

static std::string from_hex(const std::string &hex)
{
    std::string out;
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2)
    {
        out.push_back(static_cast<char>(std::stoi(hex.substr(i, 2), nullptr, 16)));
    }
    return out;
}

static std::string to_hex(const std::string &data)
{
    static const char digits[] = "0123456789abcdef";
    std::string out;
    for (unsigned char c : data)
    {
        out.push_back(digits[c >> 4]);
        out.push_back(digits[c & 15]);
    }
    return out;
}

template <std::size_t N>
static std::array<std::uint8_t, N> array_from_hex(const std::string &hex)
{
    std::string raw = from_hex(hex);
    std::array<std::uint8_t, N> out{};
    for (std::size_t i = 0; i < N; ++i)
    {
        out[i] = static_cast<std::uint8_t>(raw[i]);
    }
    return out;
}

static const std::string SUNSCREEN =
    "Ladies and Gentlemen of the class of '99: If I could offer you only one tip for the future, sunscreen would be it.";

static Encryption::Key test_key(std::uint8_t first)
{
    Encryption::Key key;
    for (std::size_t i = 0; i < key.size(); ++i)
    {
        key[i] = static_cast<std::uint8_t>(first + i);
    }
    return key;
}

void test_one()
{
    // RFC 8439 section 2.4.2: ChaCha20 encryption
    Encryption::Key key = test_key(0x00);
    Encryption::Nonce nonce = array_from_hex<12>("000000000000004a00000000");
    std::string ciphertext = Encryption::chacha20_xor(key, 1, nonce, SUNSCREEN);
    assert(to_hex(ciphertext) ==
           "6e2e359a2568f98041ba0728dd0d6981e97e7aec1d4360c20a27afccfd9fae0bf91b65c5524733ab8f593dabcd62b357"
           "1639d624e65152ab8f530c359f0861d807ca0dbf500d6a6156a38e088a22b65e52bc514d16ccf806818ce91ab7793736"
           "5af90bbf74a35be6b40b8eedf2785e42874d");
    // XOR with the same keystream decrypts
    assert(Encryption::chacha20_xor(key, 1, nonce, ciphertext) == SUNSCREEN);
    std::cout << "Test One Pass" << std::endl;
}

void test_two()
{
    // RFC 8439 section 2.5.2: Poly1305
    auto one_time_key = array_from_hex<32>("85d6be7857556d337f4452fe42d506a80103808afb0db2fd4abff6af4149f51b");
    auto tag = Encryption::poly1305(one_time_key, "Cryptographic Forum Research Group");
    assert(to_hex(std::string(reinterpret_cast<const char *>(tag.data()), tag.size())) == "a8061dc1305136c6c22b8baf0c0127a9");
    std::cout << "Test Two Pass" << std::endl;
}

void test_three()
{
    // RFC 8439 section 2.8.2: AEAD_CHACHA20_POLY1305
    Encryption::Key key = test_key(0x80);
    Encryption::Nonce nonce = array_from_hex<12>("070000004041424344454647");
    std::string aad = from_hex("50515253c0c1c2c3c4c5c6c7");
    std::string sealed = Encryption::aead_seal(key, nonce, aad, SUNSCREEN);
    assert(to_hex(sealed.substr(0, sealed.size() - 16)) ==
           "d31a8d34648e60db7b86afbc53ef7ec2a4aded51296e08fea9e2b5a736ee62d63dbea45e8ca9671282fafb69da92728b"
           "1a71de0a9e060b2905d6a5b67ecd3b3692ddbd7f2d778b8c9803aee328091b58fab324e4fad675945585808b4831d7bc"
           "3ff4def08e4b7a9de576d26586cec64b6116");
    assert(to_hex(sealed.substr(sealed.size() - 16)) == "1ae10b594f09e26a7e902ecbd0600691");

    std::string opened;
    assert(Encryption::aead_open(key, nonce, aad, sealed, opened) && opened == SUNSCREEN);

    // Any changed byte (ciphertext, tag or AAD) must be rejected
    std::string tampered = sealed;
    tampered[5] ^= 0x01;
    assert(!Encryption::aead_open(key, nonce, aad, tampered, opened));
    tampered = sealed;
    tampered[tampered.size() - 1] ^= 0x80;
    assert(!Encryption::aead_open(key, nonce, aad, tampered, opened));
    assert(!Encryption::aead_open(key, nonce, aad + "x", sealed, opened));
    std::cout << "Test Three Pass" << std::endl;
}

void test_four()
{
    // Base64 (RFC 4648 test vectors)
    const char *plain[] = {"", "f", "fo", "foo", "foob", "fooba", "foobar"};
    const char *encoded[] = {"", "Zg==", "Zm8=", "Zm9v", "Zm9vYg==", "Zm9vYmE=", "Zm9vYmFy"};
    for (int i = 0; i < 7; ++i)
    {
        assert(Encryption::base64_encode(plain[i]) == encoded[i]);
        std::string decoded;
        assert(Encryption::base64_decode(encoded[i], decoded) && decoded == plain[i]);
    }
    std::string ignored;
    assert(!Encryption::base64_decode("Zm9", ignored));    // wrong length
    assert(!Encryption::base64_decode("Zm9'", ignored));   // invalid character
    assert(!Encryption::base64_decode("Z=9v", ignored));   // padding in the middle
    std::cout << "Test Four Pass" << std::endl;
}

void test_five()
{
    // encrypt/decrypt round trip, including empty and non-ASCII (UTF-8) passwords
    Encryption cipher(test_key(0x10));
    for (std::string secret : {std::string(""), std::string("hunter2"), std::string("p\xC3\xA4ssw\xC3\xB6rd \xE2\x9C\x93"), std::string(500, 'x')})
    {
        std::string stored = cipher.encrypt(secret);
        assert(stored.rfind(Encryption::PREFIX, 0) == 0);
        assert(stored.find('\'') == std::string::npos); // safe inside the core's SQL strings
        assert(stored.find(secret) == std::string::npos || secret.empty());
        assert(cipher.decrypt(stored) == secret);
    }

    // A fresh nonce every time: the same password never encrypts to the same text
    assert(cipher.encrypt("same password") != cipher.encrypt("same password"));
    std::cout << "Test Five Pass" << std::endl;
}

void test_six()
{
    // Values that must NOT decrypt: wrong key, tampering, plain text, garbage
    Encryption cipher(test_key(0x10));
    Encryption other(test_key(0x11));
    std::string stored = cipher.encrypt("top secret");
    std::string out;

    assert(!other.try_decrypt(stored, out));                     // wrong key
    std::string tampered = stored;
    tampered[tampered.size() - 3] = (tampered[tampered.size() - 3] == 'A') ? 'B' : 'A';
    assert(!cipher.try_decrypt(tampered, out));                  // modified
    assert(!cipher.try_decrypt("top secret", out));              // legacy plain text
    assert(!cipher.try_decrypt("ENC1:", out));                   // too short
    assert(!cipher.try_decrypt("ENC1:not base64!", out));        // garbage

    bool threw = false;
    try
    {
        cipher.decrypt("top secret");
    }
    catch (const Encryption::DecryptionError &)
    {
        threw = true;
    }
    assert(threw);
    std::cout << "Test Six Pass" << std::endl;
}

void test_seven()
{
    // Key file: created on first use, reused afterwards, never overwritten, rejected if invalid
    namespace fs = std::filesystem;
    fs::path dir = fs::temp_directory_path() / "password_manager_encryption_test";
    fs::remove_all(dir);
    fs::create_directories(dir);
    std::string db_path = (dir / "test.db").string();
    std::string key_path = Encryption::key_file_path(db_path);

    assert(!fs::exists(key_path));
    std::string stored = Encryption::from_key_file(db_path).encrypt("persisted");
    assert(fs::exists(key_path));

    std::ifstream in(key_path);
    std::string contents((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();
    assert(contents.size() == 65); // 64 hex characters + newline

    // A second load (e.g. the next time the app starts) uses the same key
    assert(Encryption::from_key_file(db_path).decrypt(stored) == "persisted");
    std::ifstream again(key_path);
    std::string contents_after((std::istreambuf_iterator<char>(again)), std::istreambuf_iterator<char>());
    again.close();
    assert(contents_after == contents);

    // A corrupted key file is an error, not a silent new key
    {
        std::ofstream bad(key_path, std::ios::trunc);
        bad << "not a key";
    }
    bool threw = false;
    try
    {
        Encryption::from_key_file(db_path);
    }
    catch (const std::runtime_error &)
    {
        threw = true;
    }
    assert(threw);

    fs::remove_all(dir);
    std::cout << "Test Seven Pass" << std::endl;
}

int main()
{
    test_one();
    test_two();
    test_three();
    test_four();
    test_five();
    test_six();
    test_seven();
    return 0;
}
