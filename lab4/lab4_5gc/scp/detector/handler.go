package detector

import (
	"encoding/hex"
	"net/http"
	"strings"

	"github.com/mitchellh/mapstructure"

	"github.com/free5gc/http_wrapper"
	"github.com/free5gc/openapi/models"
	"github.com/free5gc/scp/consumer"
	"github.com/free5gc/scp/logger"
)

const (
	ERR_MANDATORY_ABSENT = "Mandatory type is absent"
	ERR_MISS_CONDITION   = "Miss condition"
	ERR_VALUE_INCORRECT  = "Unexpected value is received"
)

// Global variables
var ServingNetworkName string
var SupiOrSuci string
var rand string
var supi string
var suci string
var ck []byte
var ik []byte
var xres []byte
var autn []byte
var sqnXorAk []byte

/*
	* Refer to 29.509 5.2.2.2.2 to know the 5g AKA relationships between AMF and AUSF.
	* The following data structure can refer to these files.
	1. AuthenticationInfo can refer to openapi/models/model_authentication_info.go
	2. Response's type is models.UeAuthenticationCtx, which can refer to openapi/models/model_ue_authentication_ctx.go
	3. AuthType can refer to openapi/models/model_auth_type.go
	4. Links can refer to openapi/models/model_links_value_schema.go
	5. response.Var5gAuthData's type is models.Av5gAka, which can refer to openapi/models/model_av5g_aka.go
*/
func HandleUeAuthPostRequest(request *http_wrapper.Request) *http_wrapper.Response {
	logger.DetectorLog.Infof("HandleUeAuthPostRequest")
	updateAuthenticationInfo := request.Body.(models.AuthenticationInfo)
	ServingNetworkName = updateAuthenticationInfo.ServingNetworkName

	// NOTE: The request from AMF is guaranteed to be correct

	// TODO: Send request to target NF by setting correct uri
	// AMF -> AUSF.
	targetNfUri := "http://127.0.0.9:8000"

	// TODO: Check IEs in response body is correct
	// The function will be blocked until UDM and UDR finish their tasks.
	response, respHeader, problemDetails, err := consumer.SendUeAuthPostRequest(targetNfUri, &updateAuthenticationInfo)

	// for UEAuthenticationCtx, refer to 29.509 6.1.6.2.3.
	// for AuthType
	if response.AuthType == "" {
		logger.DetectorLog.Errorln("UEAuthenticationCtx.AuthType: " + ERR_MANDATORY_ABSENT)
		response.AuthType = models.AuthType__5_G_AKA
	} else if response.AuthType != models.AuthType__5_G_AKA &&
		response.AuthType != models.AuthType_EAP_AKA_PRIME &&
		response.AuthType != models.AuthType_EAP_TLS {
		logger.DetectorLog.Errorln("UEAuthenticationCtx.AuthType: " + ERR_VALUE_INCORRECT)
		response.AuthType = models.AuthType__5_G_AKA
	}

	// for Links
	_, isExist := response.Links["5g-aka"]

	href := "http://127.0.0.9:8000/nausf-auth/v1/ue-authentications/" + suci + "/5g-aka-confirmation"
	if !isExist {
		logger.DetectorLog.Errorln("UEAuthenticationCtx._links: " + ERR_MANDATORY_ABSENT)
		response.Links = map[string]models.LinksValueSchema{
			"5g-aka": models.LinksValueSchema{Href: href},
		}
	}

	// for Var5gAuthData(5gAuthData), refer to 29.509 6.1.6.2.5
	if response.AuthType == models.AuthType__5_G_AKA {
		var var5gAuthData models.Av5gAka
		mapstructure.Decode(response.Var5gAuthData, &var5gAuthData)

		// for rand
		if var5gAuthData.Rand == "" {
			logger.DetectorLog.Errorln("UEAuthenticationCtx.5gAuthData.Av5gAka.Rand: " + ERR_MANDATORY_ABSENT)
			var5gAuthData.Rand = rand
		} else if var5gAuthData.Rand != rand {
			logger.DetectorLog.Errorln("UEAuthenticationCtx.5gAuthData.Av5gAka.Rand: " + ERR_VALUE_INCORRECT)
			var5gAuthData.Rand = rand
		}

		// for autn
		if var5gAuthData.Autn == "" {
			logger.DetectorLog.Errorln("UEAuthenticationCtx.5gAuthData.Av5gAka.Autn: " + ERR_MANDATORY_ABSENT)
			var5gAuthData.Autn = hex.EncodeToString(autn)
		} else if var5gAuthData.Autn != hex.EncodeToString(autn) {
			logger.DetectorLog.Errorln("UEAuthenticationCtx.5gAuthData.Av5gAka.Autn: " + ERR_VALUE_INCORRECT)
			var5gAuthData.Autn = hex.EncodeToString(autn)
		}

		// for HXRES*
		// for XRES*, refer to 33.501 Annex A.4
		L1_key := append(ck, ik...)
		L1_FC := string("6B")
		L1_P0 := []byte(ServingNetworkName)
		L1_P1, _ := hex.DecodeString(rand)
		L1_P2 := xres
		xResStar := retrieveXresStar(L1_key, L1_FC, L1_P0, L1_P1, L1_P2)

		// for HXRES*, refer to 33.501 Annex A.5
		L2_P0, _ := hex.DecodeString(rand)
		L2_P1 := xResStar[:]
		L2_key := append(L2_P0, L2_P1...)
		hxResStar := hex.EncodeToString(retrieveHxresStar(L2_key))

		if var5gAuthData.HxresStar == "" {
			logger.DetectorLog.Errorln("UEAuthenticationCtx.5gAuthData.Av5gAka.HxresStar: " + ERR_MANDATORY_ABSENT)
			var5gAuthData.HxresStar = hxResStar
		} else if var5gAuthData.HxresStar != hxResStar {
			logger.DetectorLog.Errorln("UEAuthenticationCtx.5gAuthData.Av5gAka.HxresStar: " + ERR_VALUE_INCORRECT)
			var5gAuthData.HxresStar = hxResStar
		}

		response.Var5gAuthData = var5gAuthData
	}

	if response != nil {
		return http_wrapper.NewResponse(http.StatusCreated, respHeader, response)
	} else if problemDetails != nil {
		return http_wrapper.NewResponse(int(problemDetails.Status), nil, problemDetails)
	}
	logger.DetectorLog.Errorln(err)
	problemDetails = &models.ProblemDetails{
		Status: http.StatusForbidden,
		Cause:  "UNSPECIFIED",
	}
	return http_wrapper.NewResponse(http.StatusForbidden, nil, problemDetails)
}

/*
	* Refer to 29.509 5.2.2.2.2 to know the 5g AKA relationships between AMF and AUSF.
	* The following data structure can refer to these files.
	1. models.ConfirmationData can refer to openapi/models/model_confirmation_data.go
	2. Response's type is models.ConfirmationDataResponse, which can refer to openapi/models/model_confirmation_data_response.go
	3. AuthResult can refer to openapi/models/model_auth_result.go
*/
func HandleAuth5gAkaComfirmRequest(request *http_wrapper.Request) *http_wrapper.Response {
	logger.DetectorLog.Infof("Auth5gAkaComfirmRequest")
	updateConfirmationData := request.Body.(models.ConfirmationData)
	ConfirmationDataResponseID := request.Params["authCtxId"]

	// NOTE: The request from AMF is guaranteed to be correct

	// TODO: Send request to target NF by setting correct uri
	// AMF -> AUSF.
	targetNfUri := "http://127.0.0.9:8000"
	response, problemDetails, err := consumer.SendAuth5gAkaConfirmRequest(targetNfUri, ConfirmationDataResponseID, &updateConfirmationData)

	// TODO: Check IEs in response body is correct
	// for ConfirmationDataResponse, refer to 29.509 6.1.6.2.8.
	// for AuthResult
	if response.AuthResult == "" {
		logger.DetectorLog.Errorln("ConfirmationDataResponse.AuthResult: " + ERR_MANDATORY_ABSENT)
	} else if response.AuthResult != models.AuthResult_SUCCESS &&
		response.AuthResult != models.AuthResult_FAILURE &&
		response.AuthResult != models.AuthResult_ONGOING {
		logger.DetectorLog.Errorln("ConfirmationDataResponse.AuthResult: " + ERR_VALUE_INCORRECT)
	} else if response.AuthResult == models.AuthResult_SUCCESS {
		// for Supi
		if response.Supi == "" {
			logger.DetectorLog.Errorln("ConfirmationDataResponse.AuthResult.Supi: " + ERR_MISS_CONDITION)
			response.Supi = supi
		} else if response.Supi != supi {
			logger.DetectorLog.Errorln("ConfirmationDataResponse.AuthResult.Supi: " + ERR_VALUE_INCORRECT)
			response.Supi = supi
		}

		// for Kausf, refer to 33.501 Annex A.2.
		L1_key := append(ck, ik...)
		L1_FC := string("6A")
		L1_P0 := []byte(ServingNetworkName)
		L1_P1 := sqnXorAk
		kausf := retrieve5GAkaKausf(L1_key, L1_FC, L1_P0, L1_P1)

		// for Kseaf, refer to 33.501 Annex A.6.
		L2_FC := string("6A")
		L2_P0 := []byte(ServingNetworkName)
		kseaf := hex.EncodeToString(retrieveKseaf(kausf, L2_FC, L2_P0))

		if response.Kseaf == "" {
			logger.DetectorLog.Errorln("ConfirmationDataResponse.AuthResult.Kseaf: " + ERR_MISS_CONDITION)
			response.Kseaf = kseaf
		} else if response.Kseaf != kseaf {
			logger.DetectorLog.Errorln("ConfirmationDataResponse.AuthResult.Kseaf: " + ERR_VALUE_INCORRECT)
			response.Kseaf = kseaf
		}
	}

	if response != nil {
		return http_wrapper.NewResponse(http.StatusOK, nil, response)
	} else if problemDetails != nil {
		return http_wrapper.NewResponse(int(problemDetails.Status), nil, problemDetails)
	}
	logger.DetectorLog.Errorln(err)
	problemDetails = &models.ProblemDetails{
		Status: http.StatusForbidden,
		Cause:  "UNSPECIFIED",
	}
	return http_wrapper.NewResponse(http.StatusForbidden, nil, problemDetails)
}

/*
	* Refer to 29.503 5.4.2.2.2 to know the 5g AKA relationships between AUSF and UDM.
	* The following data structure can refer to these files.
	1. Request's type is models.AuthenticationInfoRequest, which can refer to openapi/models/model_authentication_info_request.go
	2. Response's type is models.AuthenticationInfoResult, which can refer to openapi/models/model_authentication_info_result.go
	3. AuthType can refer to openapi/models/model_auth_type.go
	4. AuthenticationVector can can refer to openapi/models/model_authentication_vector.go
		a. AvType can refer to openapi/models/model_av_type.go
*/
func HandleGenerateAuthDataRequest(request *http_wrapper.Request) *http_wrapper.Response {
	logger.DetectorLog.Infoln("HandleGenerateAuthDataRequest")

	authInfoRequest := request.Body.(models.AuthenticationInfoRequest)
	supiOrSuci := request.Params["supiOrSuci"]

	// TODO: Check IEs in request body is correct
	// for AuthenticationInfoRequest, refer to 29.503 6.3.6.2.2.
	// for ServingNetworkName
	if authInfoRequest.ServingNetworkName == "" {
		logger.DetectorLog.Errorln("AuthenticationInfoRequest.ServingNetworkName: " + ERR_MANDATORY_ABSENT)
		authInfoRequest.ServingNetworkName = ServingNetworkName
	}
	// for ausfInstanceId
	if authInfoRequest.AusfInstanceId == "" {
		logger.DetectorLog.Errorln("AuthenticationInfoRequest.AusfInstanceId: " + ERR_MANDATORY_ABSENT)
	}

	// TODO: Send request to target NF by setting correct uri
	// AUSF -> UDM
	targetNfUri := "http://127.0.0.3:8000"
	response, problemDetails, err := consumer.SendGenerateAuthDataRequest(targetNfUri, supiOrSuci, &authInfoRequest)

	// retrieve basic derive factors(according to this, I assume that AuthenticationVector is nonempty)
	xres, sqnXorAk, ck, ik, autn = retrieveBasicDeriveFactor(&CurrentAuthProcedure.AuthSubsData, response.AuthenticationVector.Rand)
	rand = response.AuthenticationVector.Rand
	_, _, _, _, _ = xres, sqnXorAk, ck, ik, autn

	// TODO: Check IEs in response body is correct
	// for AuthenticationInfoResult, refer to 29.503 6.3.6.2.3.

	// for AuthType
	if response.AuthType == "" {
		logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthType: " + ERR_MANDATORY_ABSENT)
		response.AuthType = models.AuthType__5_G_AKA
	}

	// for Supi
	suci = strings.Split(request.URL.Path, "/")[3]
	supi, _ = extractSupi(suci)
	if response.Supi == "" {
		logger.DetectorLog.Errorln("AuthenticationInfoResult.Supi: " + ERR_MISS_CONDITION)
		response.Supi = supi
	} else if response.Supi != supi {
		logger.DetectorLog.Errorln("AuthenticationInfoResult.Supi: " + ERR_VALUE_INCORRECT)
		response.Supi = supi
	}

	// for AuthenticationVector, refer to  6.3.6.2.8, 6.3.6.2.5
	if response.AuthType == models.AuthType__5_G_AKA {
		authenticationVector := response.AuthenticationVector

		// for avType
		if authenticationVector.AvType == "" {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.AvType: " + ERR_MANDATORY_ABSENT)
		}

		// for rand
		if authenticationVector.Rand == "" {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.Rand: " + ERR_MANDATORY_ABSENT)
			authenticationVector.Rand = rand
		} else if authenticationVector.Rand != rand {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.Rand: " + ERR_VALUE_INCORRECT)
			authenticationVector.Rand = rand
		}

		// generate XRES*, refer to 33.501 Annex A.4
		L1_key := append(ck, ik...)
		L1_FC := string("6B")
		L1_P0 := []byte(ServingNetworkName)
		L1_P1, _ := hex.DecodeString(rand)
		L1_P2 := xres
		xResStar := hex.EncodeToString(retrieveXresStar(L1_key, L1_FC, L1_P0, L1_P1, L1_P2))

		if authenticationVector.XresStar == "" {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.XresStar: " + ERR_MANDATORY_ABSENT)
			authenticationVector.XresStar = xResStar
		} else {
			if xResStar != authenticationVector.XresStar {
				logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthencticationVector.XresStar: " + ERR_VALUE_INCORRECT)
				authenticationVector.XresStar = xResStar
			}
		}

		// for autn
		if authenticationVector.Autn == "" {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.Autn: " + ERR_MANDATORY_ABSENT)
			authenticationVector.Autn = hex.EncodeToString(autn)
		} else if authenticationVector.Autn != hex.EncodeToString(autn) {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.Autn: " + ERR_VALUE_INCORRECT)
			authenticationVector.Autn = hex.EncodeToString(autn)
		}

		// for Kausf, refer to 33.501 Annex A.2.
		L2_key := append(ck, ik...)
		L2_FC := string("6A")
		L2_P0 := []byte(ServingNetworkName)
		L2_P1 := sqnXorAk
		kausf := hex.EncodeToString(retrieve5GAkaKausf(L2_key, L2_FC, L2_P0, L2_P1))

		if authenticationVector.Kausf == "" {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthenticationVector.Kausf: " + ERR_MANDATORY_ABSENT)
			authenticationVector.Kausf = kausf
		} else if kausf != authenticationVector.Kausf {
			logger.DetectorLog.Errorln("AuthenticationInfoResult.AuthencticationVector.Kausf: " + ERR_VALUE_INCORRECT)
			authenticationVector.Kausf = kausf
		}

		response.AuthenticationVector = authenticationVector
	}

	if response != nil {
		return http_wrapper.NewResponse(http.StatusOK, nil, response)
	} else if problemDetails != nil {
		return http_wrapper.NewResponse(int(problemDetails.Status), nil, problemDetails)
	}
	logger.DetectorLog.Errorln(err)
	problemDetails = &models.ProblemDetails{
		Status: http.StatusForbidden,
		Cause:  "UNSPECIFIED",
	}
	return http_wrapper.NewResponse(http.StatusForbidden, nil, problemDetails)
}

func HandleQueryAuthSubsData(request *http_wrapper.Request) *http_wrapper.Response {
	logger.DetectorLog.Infof("HandleQueryAuthSubsData")

	ueId := request.Params["ueId"]

	// TODO: Send request to correct NF by setting correct uri
	// UDM -> UDR.
	targetNfUri := "http://127.0.0.4:8000"
	response, problemDetails, err := consumer.SendAuthSubsDataGet(targetNfUri, ueId)

	// NOTE: The response from UDR is guaranteed to be correct
	CurrentAuthProcedure.AuthSubsData = *response

	if response != nil {
		return http_wrapper.NewResponse(http.StatusOK, nil, response)
	} else if problemDetails != nil {
		return http_wrapper.NewResponse(int(problemDetails.Status), nil, problemDetails)
	}
	logger.DetectorLog.Errorln(err)
	problemDetails = &models.ProblemDetails{
		Status: http.StatusForbidden,
		Cause:  "UNSPECIFIED",
	}
	return http_wrapper.NewResponse(http.StatusForbidden, nil, problemDetails)
}
