/*******************************************************************************
*   (c) 2018 ZondaX GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

package main

import (
	"testing"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/require"
	cosmosapi "github.com/ZondaX/ledger-cosmos-go"
	"fmt"
)

func Test_GetVersion(t *testing.T) {
	validatorApp, _ := cosmosapi.FindLedgerCosmosValidator()

	version, err := validatorApp.GetVersion()
	require.Nil(t, err, "Detected error")
	assert.Equal(t, uint8(0xFF), version.AppId, "TESTING MODE NOT ENABLED")
	assert.Equal(t, uint8(0x0), version.Major, "Wrong Major version")
	assert.Equal(t, uint8(0x0), version.Minor, "Wrong Minor version")
	assert.Equal(t, uint8(0x1), version.Patch, "Wrong Patch version")
}

func Test_GetPublicKey(t *testing.T) {
	validatorApp, _ := cosmosapi.FindLedgerCosmosValidator()

	path := []uint32{44, 60, 0, 0, 0}
	pubKey, err := validatorApp.GetPublicKeyED25519(path)
	require.Nil(t, err, "Detected error, err: %s\n", err)

	assert.Equal(
		t,
		32,
		len(pubKey),
		"Public key has wrong length: %x, expected length: %x\n", pubKey, 65)
}

func Test_SignED25519(t *testing.T) {
	validatorApp, _ := cosmosapi.FindLedgerCosmosValidator()

	path := []uint32{44, 60, 0, 0, 0}
	message := []byte("{\"height\":0,\"other\":\"Some dummy data\",\"round\":0}")
	_, err := validatorApp.SignED25519(path, message)
	if err != nil {
		fmt.Printf("[Sign] Error: %s\n", err)
	}

	fmt.Printf("\nSending next message:\n")
	message = []byte("{\"height\":1,\"other\":\"Some dummy data\",\"round\":0}")
	_, err = validatorApp.SignED25519(path, message)
	if err != nil {
		fmt.Printf("[Sign] Error: %s\n", err)
	}

	fmt.Printf("\nSending next message:\n")
	message = []byte("{\"height\":2,\"other\":\"Some dummy data\",\"round\":0}")
	_, err = validatorApp.SignED25519(path, message)
	if err != nil {
		fmt.Printf("[Sign] Error: %s\n", err)
	}
}

// TODO: Write a test where blocks out of order are rejected

// TODO: Write a test where we can benchmark signature times

// TODO: Write a test where the signature is tested
