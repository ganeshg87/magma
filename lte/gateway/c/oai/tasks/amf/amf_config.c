/**
 * Copyright 2020 The Magma Authors.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <libconfig.h>
#include "log.h"
#include "3gpp_24.501.h"
#include "amf_config.h"
#include "amf_default_values.h"
#include "mme_config.h"
#include "TrackingAreaIdentity.h"
#include "dynamic_memory_check.h"
#include "assertions.h"

static bool parse_bool(const char* str);

struct amf_config_s amf_config = {.rw_lock = PTHREAD_RWLOCK_INITIALIZER, 0};

/***************************************************************************
**                                                                        **
** Name:    log_amf_config_init()                                         **
**                                                                        **
** Description: Initializes log level of AMF                              **
**                                                                        **
**                                                                        **
***************************************************************************/
void log_amf_config_init(log_config_t* log_conf) {
  log_conf->ngap_log_level    = MAX_LOG_LEVEL;
  log_conf->nas_amf_log_level = MAX_LOG_LEVEL;
  log_conf->amf_app_log_level = MAX_LOG_LEVEL;
}

/***************************************************************************
**                                                                        **
** Name:    nas5g_config_init()                                           **
**                                                                        **
** Description: Initializes default values for NAS5G                      **
**                                                                        **
**                                                                        **
***************************************************************************/
void nas5g_config_init(nas5g_config_t* nas_conf) {
  nas_conf->t3502_min               = T3502_DEFAULT_VALUE;
  nas_conf->t3512_min               = T3512_DEFAULT_VALUE;
  nas_conf->t3522_sec               = T3522_DEFAULT_VALUE;
  nas_conf->t3550_sec               = T3550_DEFAULT_VALUE;
  nas_conf->t3560_sec               = T3560_DEFAULT_VALUE;
  nas_conf->t3570_sec               = T3570_DEFAULT_VALUE;
  nas_conf->t3585_sec               = T3585_DEFAULT_VALUE;
  nas_conf->t3586_sec               = T3586_DEFAULT_VALUE;
  nas_conf->t3589_sec               = T3589_DEFAULT_VALUE;
  nas_conf->t3595_sec               = T3595_DEFAULT_VALUE;
  nas_conf->force_reject_tau        = true;
  nas_conf->force_reject_sr         = true;
  nas_conf->disable_esm_information = false;
}

/***************************************************************************
**                                                                        **
** Name:    guamfi_config_init()                                          **
**                                                                        **
** Description: Initializes default values for guamfi                     **
**                                                                        **
**                                                                        **
***************************************************************************/
void guamfi_config_init(guamfi_config_t* guamfi_conf) {
  guamfi_conf->nb                        = 1;
  guamfi_conf->guamfi[0].amf_set_id      = AMFC;
  guamfi_conf->guamfi[0].amf_regionid    = AMFGID;
  guamfi_conf->guamfi[0].amf_pointer     = AMFPOINTER;
  guamfi_conf->guamfi[0].plmn.mcc_digit1 = 0;
  guamfi_conf->guamfi[0].plmn.mcc_digit2 = 0;
  guamfi_conf->guamfi[0].plmn.mcc_digit3 = 1;
  guamfi_conf->guamfi[0].plmn.mcc_digit1 = 0;
  guamfi_conf->guamfi[0].plmn.mcc_digit2 = 1;
  guamfi_conf->guamfi[0].plmn.mcc_digit3 = 0x0F;
}

/***************************************************************************
**                                                                        **
** Name:   m5g_served_tai_config_init()                                   **
**                                                                        **
** Description: Initializes default values for served_tai                 **
**                                                                        **
**                                                                        **
***************************************************************************/
void m5g_served_tai_config_init(m5g_served_tai_t* served_tai) {
  served_tai->nb_tai          = 1;
  served_tai->plmn_mcc        = calloc(1, sizeof(*served_tai->plmn_mcc));
  served_tai->plmn_mnc        = calloc(1, sizeof(*served_tai->plmn_mnc));
  served_tai->plmn_mnc_len    = calloc(1, sizeof(*served_tai->plmn_mnc_len));
  served_tai->tac             = calloc(1, sizeof(*served_tai->tac));
  served_tai->plmn_mcc[0]     = PLMN_MCC;
  served_tai->plmn_mnc[0]     = PLMN_MNC;
  served_tai->plmn_mnc_len[0] = PLMN_MNC_LEN;
  served_tai->tac[0]          = PLMN_TAC;
}

/***************************************************************************
**                                                                        **
** Name:    ngap_config_init()                                            **
**                                                                        **
** Description: Initializes default values for NGAP                       **
**                                                                        **
**                                                                        **
***************************************************************************/
void ngap_config_init(ngap_config_t* ngap_conf) {
  ngap_conf->port_number            = NGAP_PORT_NUMBER;
  ngap_conf->outcome_drop_timer_sec = NGAP_OUTCOME_TIMER_DEFAULT;
}

/***************************************************************************
**                                                                        **
** Name:    amf_config_init()                                             **
**                                                                        **
** Description: Initializes default values for AMF                        **
**                                                                        **
**                                                                        **
***************************************************************************/
void amf_config_init(amf_config_t* config) {
  memset(config, 0, sizeof(*config));

  pthread_rwlock_init(&config->rw_lock, NULL);

  config->max_gnbs                       = 2;
  config->max_ues                        = 2;
  config->unauthenticated_imsi_supported = 0;
  config->relative_capacity              = RELATIVE_CAPACITY;
  config->amf_statistic_timer            = AMF_STATISTIC_TIMER_S;
  config->use_stateless                  = false;
  ngap_config_init(&config->ngap_config);
  nas5g_config_init(&config->nas_config);
  guamfi_config_init(&config->guamfi);
  m5g_served_tai_config_init(&config->served_tai);
}

/***************************************************************************
**                                                                        **
** Name:    amf_config_parse_opt_line()                                   **
**                                                                        **
** Description: Invokes amf_config_init() to initialize                   **
**              default values of AMF                                     **
**                                                                        **
**                                                                        **
***************************************************************************/
int amf_config_parse_opt_line(
    int argc, char* argv[],
    amf_config_t* config_aA) {  // This function will be removed in upcoming PR
  amf_config_init(config_aA);
  return 0;
}

/***************************************************************************
**                                                                        **
** Name:    amf_config_parse_opt_line()                                   **
**                                                                        **
** Description: Invokes amf_config_init() to initialize                   **
**              default values of AMF                                     **
**                                                                        **
**                                                                        **
***************************************************************************/
int amf_config_parse_file(amf_config_t* config_aA) {
  config_t cfg                  = {0};
  config_setting_t* setting_mme = NULL;
  config_setting_t* setting     = NULL;
  config_setting_t* subsetting  = NULL;
  config_setting_t* sub2setting = NULL;
  int aint                      = 0;
  int i = 0, n = 0, stop_index = 0, num = 0;
  const char* astring = NULL;
  const char* tac     = NULL;
  const char* mcc     = NULL;
  const char* mnc     = NULL;
#if !EMBEDDED_SGW
  char* sgw_ip_address_for_s11 = NULL;
#endif
  bool swap                = false;
  const char* imsi_prefix  = NULL;
  const char* apn_override = NULL;

  config_init(&cfg);

  if (config_aA->config_file != NULL) {
    /*
     * Read the file. If there is an error, report it and exit.
     */
    if (!config_read_file(&cfg, bdata(config_aA->config_file))) {
      OAILOG_CRITICAL(
          LOG_CONFIG, "Failed to parse MME configuration file: %s:%d - %s\n",
          bdata(config_aA->config_file), config_error_line(&cfg),
          config_error_text(&cfg));
      config_destroy(&cfg);
      AssertFatal(
          1 == 0, "Failed to parse MME configuration file %s!\n",
          bdata(config_aA->config_file));
    }
  } else {
    config_destroy(&cfg);
    AssertFatal(0, "No MME configuration file provided!\n");
  }

  setting_mme = config_lookup(&cfg, MME_CONFIG_STRING_MME_CONFIG);

  if (setting_mme != NULL) {
    // LOGGING setting
    setting = config_setting_get_member(setting_mme, LOG_CONFIG_STRING_LOGGING);

    if (setting != NULL) {
      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_OUTPUT, (const char**) &astring)) {
        if (astring != NULL) {
          if (config_aA->log_config.output) {
            bassigncstr(config_aA->log_config.output, astring);
          } else {
            config_aA->log_config.output = bfromcstr(astring);
          }
        }
      }

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_OUTPUT_THREAD_SAFE,
              (const char**) &astring)) {
        if (astring != NULL) {
          config_aA->log_config.is_output_thread_safe = parse_bool(astring);
        }
      }

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_COLOR, (const char**) &astring)) {
        if (strcasecmp("yes", astring) == 0)
          config_aA->log_config.color = true;
        else
          config_aA->log_config.color = false;
      }

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_SCTP_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.sctp_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_S1AP_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.s1ap_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_NAS_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.nas_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_MME_APP_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.mme_app_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_S6A_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.s6a_log_level = OAILOG_LEVEL_STR2INT(astring);
      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_SECU_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.secu_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_UDP_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.udp_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_UTIL_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.util_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_ITTI_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.itti_log_level = OAILOG_LEVEL_STR2INT(astring);
#if EMBEDDED_SGW
      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_GTPV1U_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.gtpv1u_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_SPGW_APP_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.spgw_app_log_level =
            OAILOG_LEVEL_STR2INT(astring);

#else
      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_GTPV2C_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.gtpv2c_log_level = OAILOG_LEVEL_STR2INT(astring);

      if (config_setting_lookup_string(
              setting, LOG_CONFIG_STRING_S11_LOG_LEVEL,
              (const char**) &astring))
        config_aA->log_config.s11_log_level = OAILOG_LEVEL_STR2INT(astring);
#endif
      if ((config_setting_lookup_string(
              setting_mme, MME_CONFIG_STRING_ASN1_VERBOSITY,
              (const char**) &astring))) {
        if (strcasecmp(astring, MME_CONFIG_STRING_ASN1_VERBOSITY_NONE) == 0)
          config_aA->log_config.asn1_verbosity_level = 0;
        else if (
            strcasecmp(astring, MME_CONFIG_STRING_ASN1_VERBOSITY_ANNOYING) == 0)
          config_aA->log_config.asn1_verbosity_level = 2;
        else if (
            strcasecmp(astring, MME_CONFIG_STRING_ASN1_VERBOSITY_INFO) == 0)
          config_aA->log_config.asn1_verbosity_level = 1;
        else
          config_aA->log_config.asn1_verbosity_level = 0;
      }
    }

    // GENERAL MME SETTINGS
    if ((config_setting_lookup_string(
            setting_mme, MME_CONFIG_STRING_REALM, (const char**) &astring))) {
      config_aA->realm = bfromcstr(astring);
    }

    if ((config_setting_lookup_string(
            setting_mme, MME_CONFIG_STRING_FULL_NETWORK_NAME,
            (const char**) &astring))) {
      config_aA->full_network_name = bfromcstr(astring);
    }

    if ((config_setting_lookup_string(
            setting_mme, MME_CONFIG_STRING_SHORT_NETWORK_NAME,
            (const char**) &astring))) {
      config_aA->short_network_name = bfromcstr(astring);
    }

    if ((config_setting_lookup_int(
            setting_mme, MME_CONFIG_STRING_DAYLIGHT_SAVING_TIME, &aint))) {
      config_aA->daylight_saving_time = (uint32_t) aint;
    }

    if ((config_setting_lookup_string(
            setting_mme, MME_CONFIG_STRING_PID_DIRECTORY,
            (const char**) &astring))) {
      config_aA->pid_dir = bfromcstr(astring);
    }

    if ((config_setting_lookup_int(
            setting_mme, MME_CONFIG_STRING_MAXENB, &aint))) {
      config_aA->max_gnbs = (uint32_t) aint;
    }

    if ((config_setting_lookup_int(
            setting_mme, MME_CONFIG_STRING_MAXUE, &aint))) {
      config_aA->max_ues = (uint32_t) aint;
    }

    if ((config_setting_lookup_int(
            setting_mme, MME_CONFIG_STRING_RELATIVE_CAPACITY, &aint))) {
      config_aA->relative_capacity = (uint8_t) aint;
    }

    if ((config_setting_lookup_string(
            setting_mme, MME_CONFIG_STRING_USE_STATELESS,
            (const char**) &astring))) {
      config_aA->use_stateless = parse_bool(astring);
    }

    if ((config_setting_lookup_string(
            setting_mme, MME_CONFIG_STRING_UNAUTHENTICATED_IMSI_SUPPORTED,
            (const char**) &astring))) {
      config_aA->unauthenticated_imsi_supported = parse_bool(astring);
    }

#if !S6A_OVER_GRPC
    // S6A SETTING
    setting =
        config_setting_get_member(setting_mme, MME_CONFIG_STRING_S6A_CONFIG);

    if (setting != NULL) {
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_S6A_CONF_FILE_PATH,
              (const char**) &astring))) {
        if (astring != NULL) {
          if (config_aA->s6a_config.conf_file) {
            bassigncstr(config_aA->s6a_config.conf_file, astring);
          } else {
            config_aA->s6a_config.conf_file = bfromcstr(astring);
          }
        }
      }

      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_S6A_HSS_HOSTNAME,
              (const char**) &astring))) {
        if (astring != NULL) {
          if (config_aA->s6a_config.hss_host_name) {
            bassigncstr(config_aA->s6a_config.hss_host_name, astring);
          } else {
            config_aA->s6a_config.hss_host_name = bfromcstr(astring);
          }
        } else
          AssertFatal(
              1 == 0, "You have to provide a valid HSS hostname %s=...\n",
              MME_CONFIG_STRING_S6A_HSS_HOSTNAME);
      }
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_S6A_HSS_REALM,
              (const char**) &astring))) {
        if (astring != NULL) {
          if (config_aA->s6a_config.hss_realm) {
            bassigncstr(config_aA->s6a_config.hss_realm, astring);
          } else {
            config_aA->s6a_config.hss_realm = bfromcstr(astring);
          }
        } else
          AssertFatal(
              1 == 0, "You have to provide a valid HSS realm %s=...\n",
              MME_CONFIG_STRING_S6A_HSS_REALM);
      }
    }
#endif /* !S6A_OVER_GRPC */

    // TAI list setting
    setting =
        config_setting_get_member(setting_mme, MME_CONFIG_STRING_TAI_LIST);
    if (setting != NULL) {
      num = config_setting_length(setting);
      if (num < MIN_TAI_SUPPORTED) {
        fprintf(
            stderr,
            "ERROR: No TAI is configured.  At least one TAI must be "
            "configured.\n");
      }

      if (config_aA->served_tai.nb_tai != num) {
        if (config_aA->served_tai.plmn_mcc != NULL)
          free_wrapper((void**) &config_aA->served_tai.plmn_mcc);

        if (config_aA->served_tai.plmn_mnc != NULL)
          free_wrapper((void**) &config_aA->served_tai.plmn_mnc);

        if (config_aA->served_tai.plmn_mnc_len != NULL)
          free_wrapper((void**) &config_aA->served_tai.plmn_mnc_len);

        if (config_aA->served_tai.tac != NULL)
          free_wrapper((void**) &config_aA->served_tai.tac);

        config_aA->served_tai.plmn_mcc =
            calloc(num, sizeof(*config_aA->served_tai.plmn_mcc));
        config_aA->served_tai.plmn_mnc =
            calloc(num, sizeof(*config_aA->served_tai.plmn_mnc));
        config_aA->served_tai.plmn_mnc_len =
            calloc(num, sizeof(*config_aA->served_tai.plmn_mnc_len));
        config_aA->served_tai.tac =
            calloc(num, sizeof(*config_aA->served_tai.tac));
      }

      config_aA->served_tai.nb_tai = num;

      for (i = 0; i < num; i++) {
        sub2setting = config_setting_get_elem(setting, i);

        if (sub2setting != NULL) {
          if ((config_setting_lookup_string(
                  sub2setting, MME_CONFIG_STRING_MCC, &mcc))) {
            config_aA->served_tai.plmn_mcc[i] = (uint16_t) atoi(mcc);
          }

          if ((config_setting_lookup_string(
                  sub2setting, MME_CONFIG_STRING_MNC, &mnc))) {
            config_aA->served_tai.plmn_mnc[i]     = (uint16_t) atoi(mnc);
            config_aA->served_tai.plmn_mnc_len[i] = strlen(mnc);

            AssertFatal(
                (config_aA->served_tai.plmn_mnc_len[i] == MIN_MNC_LENGTH) ||
                    (config_aA->served_tai.plmn_mnc_len[i] == MAX_MNC_LENGTH),
                "Bad MNC length %u, must be %d or %d",
                config_aA->served_tai.plmn_mnc_len[i], MIN_MNC_LENGTH,
                MAX_MNC_LENGTH);
          }

          if ((config_setting_lookup_string(
                  sub2setting, MME_CONFIG_STRING_TAC, &tac))) {
            config_aA->served_tai.tac[i] = (uint16_t) atoi(tac);

            if (!TAC_IS_VALID(config_aA->served_tai.tac[i])) {
              fprintf(
                  stderr, "ERROR: Invalid TAC value " TAC_FMT,
                  config_aA->served_tai.tac[i]);
            }
          }
        }
      }

      // sort TAI list
      n = config_aA->served_tai.nb_tai;
      do {
        stop_index = 0;
        for (i = 1; i < n; i++) {
          swap = false;
          if (config_aA->served_tai.plmn_mcc[i - 1] >
              config_aA->served_tai.plmn_mcc[i]) {
            swap = true;
          } else if (
              config_aA->served_tai.plmn_mcc[i - 1] ==
              config_aA->served_tai.plmn_mcc[i]) {
            if (config_aA->served_tai.plmn_mnc[i - 1] >
                config_aA->served_tai.plmn_mnc[i]) {
              swap = true;
            } else if (
                config_aA->served_tai.plmn_mnc[i - 1] ==
                config_aA->served_tai.plmn_mnc[i]) {
              if (config_aA->served_tai.tac[i - 1] >
                  config_aA->served_tai.tac[i]) {
                swap = true;
              }
            }
          }
          if (true == swap) {
            uint16_t swap16;
            swap16 = config_aA->served_tai.plmn_mcc[i - 1];
            config_aA->served_tai.plmn_mcc[i - 1] =
                config_aA->served_tai.plmn_mcc[i];
            config_aA->served_tai.plmn_mcc[i] = swap16;

            swap16 = config_aA->served_tai.plmn_mnc[i - 1];
            config_aA->served_tai.plmn_mnc[i - 1] =
                config_aA->served_tai.plmn_mnc[i];
            config_aA->served_tai.plmn_mnc[i] = swap16;

            swap16                           = config_aA->served_tai.tac[i - 1];
            config_aA->served_tai.tac[i - 1] = config_aA->served_tai.tac[i];
            config_aA->served_tai.tac[i]     = swap16;

            stop_index = i;
          }
        }
        n = stop_index;
      } while (0 != n);

      // helper for determination of list type (global view), we could make
      // sublists with different types, but keep things simple for now
      config_aA->served_tai.list_type =
          TRACKING_AREA_IDENTITY_LIST_TYPE_ONE_PLMN_CONSECUTIVE_TACS;
      for (i = 1; i < config_aA->served_tai.nb_tai; i++) {
        if ((config_aA->served_tai.plmn_mcc[i] !=
             config_aA->served_tai.plmn_mcc[0]) ||
            (config_aA->served_tai.plmn_mnc[i] !=
             config_aA->served_tai.plmn_mnc[0])) {
          config_aA->served_tai.list_type =
              TRACKING_AREA_IDENTITY_LIST_TYPE_MANY_PLMNS;
          break;
        } else if (
            (config_aA->served_tai.plmn_mcc[i] !=
             config_aA->served_tai.plmn_mcc[i - 1]) ||
            (config_aA->served_tai.plmn_mnc[i] !=
             config_aA->served_tai.plmn_mnc[i - 1])) {
          config_aA->served_tai.list_type =
              TRACKING_AREA_IDENTITY_LIST_TYPE_MANY_PLMNS;
          break;
        }
        if (config_aA->served_tai.tac[i] !=
            (config_aA->served_tai.tac[i - 1] + 1)) {
          config_aA->served_tai.list_type =
              TRACKING_AREA_IDENTITY_LIST_TYPE_ONE_PLMN_NON_CONSECUTIVE_TACS;
        }
      }
    }

    // NAS SETTING
    setting =
        config_setting_get_member(setting_mme, MME_CONFIG_STRING_NAS_CONFIG);

    if (setting != NULL) {
      subsetting = config_setting_get_member(
          setting, MME_CONFIG_STRING_NAS_SUPPORTED_INTEGRITY_ALGORITHM_LIST);

      if (subsetting != NULL) {
        num = config_setting_length(subsetting);

        if (num <= 8) {
          for (i = 0; i < num; i++) {
            astring = config_setting_get_string_elem(subsetting, i);

            if (strcmp("EIA0", astring) == 0)
              config_aA->m5g_nas_config.prefered_integrity_algorithm[i] =
                  EIA0_ALG_ID;
            else if (strcmp("EIA1", astring) == 0)
              config_aA->m5g_nas_config.prefered_integrity_algorithm[i] =
                  EIA1_128_ALG_ID;
            else if (strcmp("EIA2", astring) == 0)
              config_aA->m5g_nas_config.prefered_integrity_algorithm[i] =
                  EIA2_128_ALG_ID;
            else
              config_aA->m5g_nas_config.prefered_integrity_algorithm[i] =
                  EIA0_ALG_ID;
          }

          for (i = num; i < 8; i++) {
            config_aA->m5g_nas_config.prefered_integrity_algorithm[i] =
                EIA0_ALG_ID;
          }
        }
      }

      subsetting = config_setting_get_member(
          setting, MME_CONFIG_STRING_NAS_SUPPORTED_CIPHERING_ALGORITHM_LIST);

      if (subsetting != NULL) {
        num = config_setting_length(subsetting);

        if (num <= 8) {
          for (i = 0; i < num; i++) {
            astring = config_setting_get_string_elem(subsetting, i);

            if (strcmp("EEA0", astring) == 0)
              config_aA->m5g_nas_config.prefered_ciphering_algorithm[i] =
                  EEA0_ALG_ID;
            else if (strcmp("EEA1", astring) == 0)
              config_aA->m5g_nas_config.prefered_ciphering_algorithm[i] =
                  EEA1_128_ALG_ID;
            else if (strcmp("EEA2", astring) == 0)
              config_aA->m5g_nas_config.prefered_ciphering_algorithm[i] =
                  EEA2_128_ALG_ID;
            else
              config_aA->m5g_nas_config.prefered_ciphering_algorithm[i] =
                  EEA0_ALG_ID;
          }

          for (i = num; i < 8; i++) {
            config_aA->m5g_nas_config.prefered_ciphering_algorithm[i] =
                EEA0_ALG_ID;
          }
        }
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3402_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3402_min = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3412_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3412_min = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3422_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3422_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3450_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3450_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3460_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3460_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3470_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3470_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3485_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3485_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3486_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3486_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3489_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3489_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_int(
              setting, MME_CONFIG_STRING_NAS_T3495_TIMER, &aint))) {
        config_aA->m5g_nas_config.t3495_sec = (uint32_t) aint;
      }
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_NAS_FORCE_REJECT_TAU,
              (const char**) &astring))) {
        config_aA->m5g_nas_config.force_reject_tau = parse_bool(astring);
      }
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_NAS_FORCE_REJECT_SR,
              (const char**) &astring))) {
        config_aA->m5g_nas_config.force_reject_sr = parse_bool(astring);
      }
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_NAS_DISABLE_ESM_INFORMATION_PROCEDURE,
              (const char**) &astring))) {
        config_aA->m5g_nas_config.disable_esm_information = parse_bool(astring);
      }
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_NAS_ENABLE_APN_CORRECTION,
              (const char**) &astring))) {
        config_aA->m5g_nas_config.enable_apn_correction = parse_bool(astring);
      }

      // Parsing APN CORRECTION MAP
      if (config_aA->m5g_nas_config.enable_apn_correction) {
        subsetting = config_setting_get_member(
            setting, MME_CONFIG_STRING_NAS_APN_CORRECTION_MAP_LIST);
        config_aA->m5g_nas_config.m5g_apn_map_config.nb = 0;
        if (subsetting != NULL) {
          num = config_setting_length(subsetting);
          OAILOG_INFO(
              LOG_MME_APP, "Number of apn correction map configured =%d\n",
              num);
          AssertFatal(
              num <= MAX_APN_CORRECTION_MAP_LIST,
              "Number of apn correction map configured:%d exceeds the maximum "
              "number supported"
              ":%d \n",
              num, MAX_APN_CORRECTION_MAP_LIST);

          for (i = 0; i < num; i++) {
            sub2setting = config_setting_get_elem(subsetting, i);
            if (sub2setting != NULL) {
              if ((config_setting_lookup_string(
                      sub2setting,
                      MME_CONFIG_STRING_NAS_APN_CORRECTION_MAP_IMSI_PREFIX,
                      (const char**) &imsi_prefix))) {
                if (config_aA->m5g_nas_config.m5g_apn_map_config.apn_map[i]
                        .imsi_prefix) {
                  bassigncstr(
                      config_aA->m5g_nas_config.m5g_apn_map_config.apn_map[i]
                          .imsi_prefix,
                      imsi_prefix);
                } else {
                  config_aA->m5g_nas_config.m5g_apn_map_config.apn_map[i]
                      .imsi_prefix = bfromcstr(imsi_prefix);
                }
              }
              if ((config_setting_lookup_string(
                      sub2setting,
                      MME_CONFIG_STRING_NAS_APN_CORRECTION_MAP_APN_OVERRIDE,
                      (const char**) &apn_override))) {
                if (config_aA->m5g_nas_config.m5g_apn_map_config.apn_map[i]
                        .apn_override) {
                  bassigncstr(
                      config_aA->m5g_nas_config.m5g_apn_map_config.apn_map[i]
                          .apn_override,
                      apn_override);
                } else {
                  config_aA->m5g_nas_config.m5g_apn_map_config.apn_map[i]
                      .apn_override = bfromcstr(apn_override);
                }
              }
              config_aA->m5g_nas_config.m5g_apn_map_config.nb += 1;
            }
          }
        }
      }
    }
#if (!EMBEDDED_SGW)
    // S-GW Setting
    setting =
        config_setting_get_member(setting_mme, MME_CONFIG_STRING_SGW_CONFIG);

    if (setting != NULL) {
      if ((config_setting_lookup_string(
              setting, MME_CONFIG_STRING_SGW_IPV4_ADDRESS_FOR_S11,
              (const char**) &sgw_ip_address_for_s11))) {
        OAILOG_DEBUG(
            LOG_MME_APP, "sgw interface IP information %s\n",
            sgw_ip_address_for_s11);

        IPV4_STR_ADDR_TO_INADDR(
            sgw_ip_address_for_s11, config_aA->e_dns_emulation.sgw_ip_addr[0],
            "BAD IP ADDRESS FORMAT FOR SGW S11 !\n");

        OAILOG_INFO(
            LOG_SPGW_APP, "Parsing configuration file found S-GW S11: %s\n",
            inet_ntoa(config_aA->e_dns_emulation.sgw_ip_addr[0]));
      }
    }
#endif
  }

  config_destroy(&cfg);
  return 0;
}

static bool parse_bool(const char* str) {
  if (strcasecmp(str, "yes") == 0) return true;
  if (strcasecmp(str, "true") == 0) return true;
  if (strcasecmp(str, "no") == 0) return false;
  if (strcasecmp(str, "false") == 0) return false;
  if (strcasecmp(str, "") == 0) return false;

  Fatal("Error in config file: got \"%s\" but expected bool\n", str);
}
