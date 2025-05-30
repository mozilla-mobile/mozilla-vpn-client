/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "testchacha20poly.h"

#include <QtTest/QtTest>

#include "chacha20poly1305.h"

void TestChaCha20Poly::rfc7539_data() {
  QTest::addColumn<QByteArray>("plaintext");
  QTest::addColumn<QByteArray>("authdata");
  QTest::addColumn<QByteArray>("key");
  QTest::addColumn<QByteArray>("nonce");
  QTest::addColumn<QByteArray>("ciphertext");
  QTest::addColumn<QByteArray>("tag");

  QTest::addRow("Test Vector for AEAD_CHACHA20_POLY1305")
    << fromHex("4c616469657320616e642047656e746c",
               "656d656e206f662074686520636c6173",
               "73206f66202739393a20496620492063",
               "6f756c64206f6666657220796f75206f",
               "6e6c79206f6e652074697020666f7220",
               "746865206675747572652c2073756e73",
               "637265656e20776f756c642062652069",
               "742e")
    << fromHex("50515253c0c1c2c3c4c5c6c7")
    << fromHex("808182838485868788898a8b8c8d8e8f",
               "909192939495969798999a9b9c9d9e9f")
    << fromHex("070000004041424344454647")
    << fromHex("d31a8d34648e60db7b86afbc53ef7ec2",
               "a4aded51296e08fea9e2b5a736ee62d6",
               "3dbea45e8ca9671282fafb69da92728b",
               "1a71de0a9e060b2905d6a5b67ecd3b36",
               "92ddbd7f2d778b8c9803aee328091b58",
               "fab324e4fad675945585808b4831d7bc",
               "3ff4def08e4b7a9de576d26586cec64b",
               "6116")
    << fromHex("1ae10b594f09e26a7e902ecbd0600691");

  QTest::addRow("ChaCha20-Poly1305 AEAD Decryption")
    << fromHex("496e7465726e65742d44726166747320",
               "61726520647261667420646f63756d65",
               "6e74732076616c696420666f72206120",
               "6d6178696d756d206f6620736978206d",
               "6f6e74687320616e64206d6179206265",
               "20757064617465642c207265706c6163",
               "65642c206f72206f62736f6c65746564",
               "206279206f7468657220646f63756d65",
               "6e747320617420616e792074696d652e",
               "20497420697320696e617070726f7072",
               "6961746520746f2075736520496e7465",
               "726e65742d4472616674732061732072",
               "65666572656e6365206d617465726961",
               "6c206f7220746f206369746520746865",
               "6d206f74686572207468616e20617320",
               "2fe2809c776f726b20696e2070726f67",
               "726573732e2fe2809d")
    << fromHex("f33388860000000000004e91")
    << fromHex("1c9240a5eb55d38af333888604f6b5f0"
               "473917c1402b80099dca5cbc207075c0")
    << fromHex("000000000102030405060708")
    << fromHex("64a0861575861af460f062c79be643bd",
               "5e805cfd345cf389f108670ac76c8cb2",
               "4c6cfc18755d43eea09ee94e382d26b0",
               "bdb7b73c321b0100d4f03b7f355894cf",
               "332f830e710b97ce98c8a84abd0b9481",
               "14ad176e008d33bd60f982b1ff37c855",
               "9797a06ef4f0ef61c186324e2b350638",
               "3606907b6a7c02b0f9f6157b53c867e4",
               "b9166c767b804d46a59b5216cde7a4e9",
               "9040c5a40433225ee282a1b0a06c523e",
               "af4534d7f83fa1155b0047718cbc546a",
               "0d072b04b3564eea1b422273f548271a",
               "0bb2316053fa76991955ebd63159434e",
               "cebb4e466dae5a1073a6727627097a10",
               "49e617d91d361094fa68f0ff77987130",
               "305beaba2eda04df997b714d6c6f2c29",
               "a6ad5cb4022b02709b")
    << fromHex("eead9d67890cbb22392336fea1851f38");
}

void TestChaCha20Poly::rfc7539() {
  QFETCH(QByteArray, plaintext);
  QFETCH(QByteArray, authdata);
  QFETCH(QByteArray, key);
  QFETCH(QByteArray, nonce);
  QFETCH(QByteArray, ciphertext);
  QFETCH(QByteArray, tag);

  Chacha20Poly1305 cipher(key);
  QCOMPARE(cipher.NonceSize, nonce.length());
  QCOMPARE(cipher.MacSize, tag.length());
  QCOMPARE(cipher.KeySize, key.length());

  // Test encryption.
  QByteArray encTag;
  QByteArray encData = cipher.encrypt(nonce, authdata, plaintext, encTag);
  QCOMPARE(encData, ciphertext);

  // Test decryption.
  QByteArray decData = cipher.decrypt(nonce, authdata, ciphertext, tag);
  QCOMPARE(decData, plaintext);
};
